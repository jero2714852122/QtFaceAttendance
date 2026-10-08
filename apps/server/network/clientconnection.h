#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QString>

class QTcpSocket;

class ClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit ClientConnection(
        QTcpSocket* socket,
        QObject* parent = nullptr);

    QString peerName() const;

    qint64 sendPayload(const QByteArray& payload);

signals:
    void messageReceived(
        const QByteArray& message);

    void disconnected(
        const QString& peer);

    void errorOccurred(
        const QString& message);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onSocketError();

private:
    void processFrames();

    QTcpSocket* socket_ = nullptr;
    QByteArray receiveBuffer_;
};
