#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include "database/employeerepository.h"

class DatabaseManager;
class Server;
class ServerWindow;

class ServerController : public QObject
{
    Q_OBJECT

public:
    ServerController(
        Server& server,
        ServerWindow& window,
        DatabaseManager& database,
        QObject* parent = nullptr);

    bool initializeDatabase();
    void loadEmployees();
private slots:
    void onClientConnected(
        const QString& peer);

    void onClientDisconnected(
        const QString& peer);

    void onClientError(
        const QString& message);

    void onMessageReceived(
        const QString& peer,
        const QByteArray& message);

    void onAddEmployee(
        const QString& employeeNo,
        const QString& name,
        const QString& department);

    void onRefreshEmployees();

    void onDeleteEmployee(
        qint64 id);

private:
    Server& server_;
    ServerWindow& window_;
    DatabaseManager& database_;
    EmployeeRepository employeeRepository_;
};
