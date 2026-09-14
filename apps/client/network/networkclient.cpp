#include "networkclient.h"

#include "frameprotocol.h"

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent)
{
    QObject::connect(
        &socket_,
        &QTcpSocket::connected,
        this,
        &NetworkClient::connected);

    QObject::connect(
        &socket_,
        &QTcpSocket::disconnected,
        this,
        &NetworkClient::disconnected);

    QObject::connect(
        &socket_,
        &QTcpSocket::errorOccurred,
        this,
        [this](QAbstractSocket::SocketError) {
            emit connectionError(socket_.errorString());
        });
}

void NetworkClient::connectToServer(const QString& host, quint16 port)
{
    socket_.connectToHost(host, port);
}

qint64 NetworkClient::sendPayload(const QByteArray& payload)
{
    if (!isConnected())
    {
        return -1;
    }

    return socket_.write(FrameProtocol::pack(payload));
}

bool NetworkClient::isConnected() const
{
    return socket_.state() == QAbstractSocket::ConnectedState;
}