#include "servercontroller.h"

#include "database/databasemanager.h"
#include "network/server.h"
#include "ui/serverwindow.h"
#include "database/employeerepository.h"
#include <QCoreApplication>
#include <QDir>

ServerController::ServerController(
    Server& server,
    ServerWindow& window,
    DatabaseManager& database,
    QObject* parent)
    : QObject(parent)
    , server_(server)
    , window_(window)
    , database_(database)
    ,employeeRepository_(database)
    ,faceTemplateRepository_(database)
    ,attendanceRepository_(database)
{
    QObject::connect(
        &server_,
        &Server::listeningStarted,
        &window_,
        &ServerWindow::appendStatusText);

    QObject::connect(
        &server_,
        &Server::listeningError,
        &window_,
        &ServerWindow::appendStatusText);

    QObject::connect(
        &server_,
        &Server::clientConnected,
        this,
        &ServerController::onClientConnected);

    QObject::connect(
        &server_,
        &Server::clientDisconnected,
        this,
        &ServerController::onClientDisconnected);

    QObject::connect(
        &server_,
        &Server::clientError,
        this,
        &ServerController::onClientError);

    QObject::connect(
        &server_,
        &Server::messageReceived,
        this,
        &ServerController::onMessageReceived);

    QObject::connect(
        &window_,
        &ServerWindow::addEmployeeRequested,
        this,
        &ServerController::onAddEmployee);

    QObject::connect(
        &window_,
        &ServerWindow::refreshEmployeesRequested,
        this,
        &ServerController::onRefreshEmployees);

    QObject::connect(
        &window_,
        &ServerWindow::deleteEmployeeRequested,
        this,
        &ServerController::onDeleteEmployee);

    QObject::connect(
        &window_,
        &ServerWindow::updateEmployeeRequested,
        this,
        &ServerController::onUpdateEmployee);

    QObject::connect(
        &window_,
        &ServerWindow::registerFaceRequested,
        this,
        &ServerController::onRegisterFace);
}

bool ServerController::initializeDatabase()
{
    const QString dataDirectory =
        QCoreApplication::applicationDirPath()
        + "/data";

    if (!QDir().mkpath(dataDirectory))
    {
        window_.appendStatusText(
            "数据库目录创建失败："
            + dataDirectory);
        return false;
    }

    const QString databasePath =
        dataDirectory
        + "/attendance.db";

    if (!database_.open(databasePath))
    {
        window_.appendStatusText(
            "数据库打开失败："
            + database_.lastError());
        return false;
    }

    if (!database_.initializeSchema())
    {
        window_.appendStatusText(
            "数据库表初始化失败："
            + database_.lastError());
        return false;
    }

    window_.appendStatusText(
        "数据库初始化成功："
        + databasePath);
    return true;
}

void ServerController::onClientConnected(
    const QString& peer)
{
    window_.appendStatusText(
        "客户端已连接：" + peer);
}

void ServerController::onClientDisconnected(
    const QString& peer)
{
    window_.appendStatusText(
        "客户端已断开：" + peer);

    // 客户端换了，上一次的识别结果和那一帧的人脸都不再代表当前画面。
    // 不清掉的话，客户端重连后画面没变，日志会漏掉这次状态刷新；
    // 而且"登记人脸"有可能把上一台机器留下的人脸登记进去。
    lastResult_ = FrameResult();
    lastEmbedding_ = cv::Mat();
}

void ServerController::onClientError(
    const QString& message)
{
    window_.appendStatusText(
        "客户端错误：" + message);
}

void ServerController::onMessageReceived(
    const QString& peer,
    const QByteArray& message)
{
    // 协议分流：JPEG\n 开头的是图像帧，其他的是文本控制消息。
    // 图像帧每秒来两帧，不能按帧记日志，所以这里不打印字节数。
    if (!message.startsWith("JPEG\n"))
    {
        window_.appendStatusText(
            peer
            + " 收到完整消息，字节数："
            + QString::number(message.size()));

        return;
    }

    std::vector<FaceEngine::Face> faces;

    if (!faceEngine_.analyzeJpeg(
            message.mid(5),
            faces))
    {
        window_.appendStatusText(
            "人脸分析失败："
            + faceEngine_.lastError());

        return;
    }

    FrameResult result;

    if (faces.empty())
    {
        result.text = "未检测到人脸";
    }
    else
    {
        // 一张画面里可能有多张脸，只处理可信度最高的那张。考勤场景
        // 本来就要求一人入镜，多张脸属于异常输入。
        const FaceEngine::Face* bestFace = nullptr;

        for (const FaceEngine::Face& face : faces)
        {
            if (bestFace == nullptr
                || face.score > bestFace->score)
            {
                bestFace = &face;
            }
        }

        const cv::Mat embedding =
            bestFace->embedding;

        // 登记功能用的永远是"最近一帧的人脸"，跟日志是否输出无关，
        // 所以每次都要更新。
        lastEmbedding_ = embedding.clone();

        result = recognize(embedding);
        result.hasFace = true;
    }

    // 拿"是谁"来判断状态变没变，不能拿那行文字。相似度每帧都在第四位
    // 小数上抖动，文字跟着变，每帧都会算成新状态，日志又会刷屏。
    const bool stateChanged =
        result.hasFace != lastResult_.hasFace
        || result.matched != lastResult_.matched
        || result.employeeId != lastResult_.employeeId;

    if (stateChanged)
    {
        window_.appendStatusText(result.text);

        QString attendanceText;

        // 考勤只在状态变化时判断一次。人一直站在镜头前不该反复记，
        // 离开再回来才算一次新的出现，那时再交给限流窗口决定。
        if (result.matched)
        {
            attendanceText = recordAttendance(
                result.employeeId,
                result.score);

            window_.appendStatusText(attendanceText);
        }

        sendResult(peer, result, attendanceText);
    }

    lastResult_ = result;
}

ServerController::FrameResult ServerController::recognize(
    const cv::Mat& embedding)
{
    FrameResult result;

    if (templates_.isEmpty())
    {
        result.text = "未识别：还没有登记过任何人脸模板";

        return result;
    }

    QString bestName;
    QString bestEmployeeNo;

    for (const FaceTemplate& candidate : templates_)
    {
        const cv::Mat candidateEmbedding =
            FaceEngine::fromBytes(
                candidate.featureData);

        // 库里可能存着尺寸不对的脏数据，跳过一条比让整次识别失败合理。
        if (candidateEmbedding.empty())
        {
            continue;
        }

        const double score =
            faceEngine_.similarity(
                embedding,
                candidateEmbedding);

        if (score > result.score)
        {
            result.score = score;
            result.employeeId = candidate.employeeId;
            bestName = candidate.name;
            bestEmployeeNo = candidate.employeeNo;
        }
    }

    if (result.score >= FaceEngine::kMatchThreshold)
    {
        result.matched = true;
        result.name = bestName;

        result.text = QString("识别成功：%1（工号 %2），相似度 %3")
            .arg(bestName)
            .arg(bestEmployeeNo)
            .arg(result.score, 0, 'f', 4);
    }
    else
    {
        result.text = QString("未识别：最高相似度 %1，低于阈值 %2")
            .arg(result.score, 0, 'f', 4)
            .arg(FaceEngine::kMatchThreshold, 0, 'f', 3);
    }

    return result;
}

QString ServerController::recordAttendance(
    qint64 employeeId,
    double confidence)
{
    // 限流窗口。没有它，人离开两秒再回来就多记一条，
    // 一天下来同一个人能被记几十次。
    constexpr int kRepeatWindowSeconds = 120;

    bool alreadyRecorded = false;

    if (!attendanceRepository_.hasRecordWithin(
            employeeId,
            kRepeatWindowSeconds,
            alreadyRecorded))
    {
        return "考勤查询失败："
            + attendanceRepository_.lastError();
    }

    if (alreadyRecorded)
    {
        return QString("考勤未记录：%1 秒内已经记过一次")
            .arg(kRepeatWindowSeconds);
    }

    // 目前只记签到。签退要等有了时间段规则再分。
    if (!attendanceRepository_.addRecord(
            employeeId,
            "check_in",
            confidence))
    {
        return "考勤写入失败："
            + attendanceRepository_.lastError();
    }

    return "考勤已记录：签到";
}

void ServerController::sendResult(
    const QString& peer,
    const FrameResult& result,
    const QString& attendanceText)
{
    QString identityText;

    if (!result.hasFace)
    {
        identityText = "画面中无人";
    }
    else if (!result.matched)
    {
        identityText = "未识别";
    }
    else
    {
        identityText = result.name;
    }

    // 回传给客户端的格式：RESULT 加两个制表符分隔的字段，身份和考勤结论。
    // 用制表符而不是换行，是因为整条消息就是一行，客户端拆起来最简单。
    QByteArray payload = "RESULT\t";
    payload.append(identityText.toUtf8());
    payload.append("\t");
    payload.append(attendanceText.toUtf8());

    server_.sendToPeer(peer, payload);
}

void ServerController::onAddEmployee(
    const QString& employeeNo,
    const QString& name,
    const QString& department)
{
    const QString cleanEmployeeNo =
        employeeNo.trimmed();

    const QString cleanName =
        name.trimmed();

    const QString cleanDepartment =
        department.trimmed();

    if (cleanEmployeeNo.isEmpty()
        || cleanName.isEmpty())
    {
        window_.appendStatusText(
            "员工编号和姓名不能为空");

        return;
    }
    Employee existing;

    if (employeeRepository_.findByEmployeeNo(
            cleanEmployeeNo,
            existing))
    {
        window_.appendStatusText(
            "新增员工失败：工号 "
            + cleanEmployeeNo
            + " 已存在");

        return;
    }
    if (!employeeRepository_.addEmployee(
            cleanEmployeeNo,
            cleanName,
            cleanDepartment))
    {
        window_.appendStatusText(
            "新增员工失败："
            + employeeRepository_.lastError());

        return;
    }

    window_.appendStatusText(
        "员工新增成功："
        + cleanName);

    loadEmployees();
}

void ServerController::onRefreshEmployees()
{
    loadEmployees();
}

void ServerController::onDeleteEmployee(
    qint64 id)
{
    if (!employeeRepository_.removeById(id))
    {
        window_.appendStatusText(
            "删除员工失败："
            + employeeRepository_.lastError());

        return;
    }

    window_.appendStatusText(
        "员工删除成功");

    loadEmployees();
}

void ServerController::onUpdateEmployee(
    qint64 id,
    const QString& name,
    const QString& department)
{
    const QString cleanName =
        name.trimmed();

    const QString cleanDepartment =
        department.trimmed();

    if (cleanName.isEmpty())
    {
        window_.appendStatusText(
            "姓名不能为空");

        return;
    }

    if (!employeeRepository_.updateEmployee(
            id,
            cleanName,
            cleanDepartment))
    {
        window_.appendStatusText(
            "员工更新失败："
            + employeeRepository_.lastError());

        return;
    }

    window_.appendStatusText(
        "员工更新成功："
        + cleanName);

    loadEmployees();
}

void ServerController::loadTemplates()
{
    QList<FaceTemplate> loaded;

    if (!faceTemplateRepository_.findAll(loaded))
    {
        window_.appendStatusText(
            "读取人脸模板失败："
            + faceTemplateRepository_.lastError());

        return;
    }

    templates_ = loaded;

    // 识别时要逐条比对，放在内存里避免每帧查一次库。
    window_.appendStatusText(
        QString("已加载 %1 张人脸模板")
            .arg(templates_.size()));
}

void ServerController::onRegisterFace(
    qint64 id)
{
    if (lastEmbedding_.empty())
    {
        window_.appendStatusText(
            "登记失败：还没有收到带人脸的画面");

        return;
    }

    // 一张脸只能属于一个工号。登记前先跟已有的模板比一遍：同一张脸挂在
    // 两个人名下，考勤就会记到错的人头上，而且事后完全没法分辨是谁。
    for (const FaceTemplate& candidate : templates_)
    {
        // 本人重新登记是覆盖，允许。
        if (candidate.employeeId == id)
        {
            continue;
        }

        const cv::Mat candidateEmbedding =
            FaceEngine::fromBytes(
                candidate.featureData);

        if (candidateEmbedding.empty())
        {
            continue;
        }

        const double score =
            faceEngine_.similarity(
                lastEmbedding_,
                candidateEmbedding);

        if (score >= FaceEngine::kMatchThreshold)
        {
            window_.appendStatusText(
                QString("登记失败：这张脸已经登记给 %1（工号 %2），相似度 %3")
                    .arg(candidate.name)
                    .arg(candidate.employeeNo)
                    .arg(score, 0, 'f', 4));

            return;
        }
    }

    if (!faceTemplateRepository_.saveTemplate(
            id,
            FaceEngine::toBytes(lastEmbedding_)))
    {
        window_.appendStatusText(
            "人脸登记失败："
            + faceTemplateRepository_.lastError());

        return;
    }

    window_.appendStatusText(
        "人脸登记成功");

    loadTemplates();
}

void ServerController::loadEmployees()
{
    QList<Employee> employees;

    if (!employeeRepository_.findAll(
            employees))
    {
        window_.appendStatusText(
            "查询员工失败："
            + employeeRepository_.lastError());
        return;
    }

    window_.setEmployees(employees);

    window_.setEmployeeCount(
        employees.size());
}
bool ServerController::initializeVision()
{
    const QString modelDirectory =
        QCoreApplication::applicationDirPath()
        + "/models";

    const QString detectorPath =
        modelDirectory
        + "/face_detection_yunet_2023mar.onnx";

    const QString recognizerPath =
        modelDirectory
        + "/face_recognition_sface_2021dec.onnx";

    if (!faceEngine_.load(
            detectorPath,
            recognizerPath))
    {
        window_.appendStatusText(
            "人脸模型加载失败："
            + faceEngine_.lastError());

        return false;
    }

    window_.appendStatusText(
        "人脸模型加载成功");

    return true;
}
