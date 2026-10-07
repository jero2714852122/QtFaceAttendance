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
    lastRecognitionResult_.clear();
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

    QString result;

    if (faces.empty())
    {
        result = "未检测到人脸";
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

        result = describeRecognition(embedding);
    }

    // 只在结果发生变化时记一行。日志记录的是状态变化，不是每一次采样，
    // 否则每秒钟六行会把真正重要的信息冲走。
    if (result != lastRecognitionResult_)
    {
        lastRecognitionResult_ = result;

        window_.appendStatusText(result);
    }
}

QString ServerController::describeRecognition(
    const cv::Mat& embedding)
{
    if (templates_.isEmpty())
    {
        return "未识别：还没有登记过任何人脸模板";
    }

    double bestScore = 0.0;
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

        if (score > bestScore)
        {
            bestScore = score;
            bestName = candidate.name;
            bestEmployeeNo = candidate.employeeNo;
        }
    }

    if (bestScore >= FaceEngine::kMatchThreshold)
    {
        return QString("识别成功：%1（工号 %2），相似度 %3")
            .arg(bestName)
            .arg(bestEmployeeNo)
            .arg(bestScore, 0, 'f', 4);
    }

    return QString("未识别：最高相似度 %1，低于阈值 %2")
        .arg(bestScore, 0, 'f', 4)
        .arg(FaceEngine::kMatchThreshold, 0, 'f', 3);
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
