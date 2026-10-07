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
    window_.appendStatusText(
        peer
        + " 收到完整消息，字节数："
        + QString::number(message.size()));

    if (!message.startsWith("JPEG\n"))
    {
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

    if (faces.empty())
    {
        window_.appendStatusText(
            "图像中未检测到人脸");

        return;
    }

    const cv::Mat& embedding =
        faces.front().embedding;

    window_.appendStatusText(
        QString("检测到人脸：位置(%1, %2)，尺寸 %3x%4，特征维度 %5x%6")
            .arg(faces.front().box.x)
            .arg(faces.front().box.y)
            .arg(faces.front().box.width)
            .arg(faces.front().box.height)
            .arg(embedding.rows)
            .arg(embedding.cols));

    window_.appendStatusText(
        QString("特征前 4 个值：%1, %2, %3, %4")
            .arg(embedding.at<float>(0, 0), 0, 'f', 4)
            .arg(embedding.at<float>(0, 1), 0, 'f', 4)
            .arg(embedding.at<float>(0, 2), 0, 'f', 4)
            .arg(embedding.at<float>(0, 3), 0, 'f', 4));

    if (!lastEmbedding_.empty())
    {
        window_.appendStatusText(
            QString("与上一帧的相似度：%1（判定阈值 0.363）")
                .arg(
                    faceEngine_.similarity(
                        lastEmbedding_,
                        embedding),
                    0,
                    'f',
                    4));
    }

    lastEmbedding_ = embedding.clone();
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
