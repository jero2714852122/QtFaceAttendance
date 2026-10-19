#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QString>
#include <QTcpServer>

class ClientConnection;

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

    bool listen(
        const QHostAddress& address,
        quint16 port);

    void close();

    // 把一条消息只回给指定的那个客户端。
    qint64 sendToPeer(
        const QString& peer,
        const QByteArray& payload);
    // 主动断开指定的客户端。找不到返回 false。
    bool disconnectPeer(
        const QString& peer);


signals:
    void listeningStarted(
        const QString& message);

    void listeningError(
        const QString& message);

    void clientConnected(
        const QString& peer);

    void clientDisconnected(
        const QString& peer);

    void messageReceived(
        const QString& peer,
        const QByteArray& message);

    void clientError(
        const QString& message);

private slots:
    void onNewConnection();

private:
    QTcpServer server_;
    QList<ClientConnection*> connections_;
};
