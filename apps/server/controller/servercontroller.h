#pragma once

#include <QObject>

class Server;
class ServerWindow;

class ServerController : public QObject
{
    Q_OBJECT

public:
    ServerController(
        Server& server,
        ServerWindow& window,
        QObject* parent = nullptr);

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
};