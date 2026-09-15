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