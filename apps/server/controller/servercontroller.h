#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

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

private:
    Server& server_;
    ServerWindow& window_;
    DatabaseManager& database_;
};
