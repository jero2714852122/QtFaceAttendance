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
        + QString::number(
            message.size()));
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
