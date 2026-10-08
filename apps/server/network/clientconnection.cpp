#include "clientconnection.h"

#include "frameprotocol.h"

#include <QTcpSocket>

ClientConnection::ClientConnection(
    QTcpSocket* socket,
    QObject* parent)
    : QObject(parent)
    , socket_(socket)
{
    socket_->setParent(this);

    QObject::connect(
        socket_,
        &QTcpSocket::readyRead,
        this,
        &ClientConnection::onReadyRead);

    QObject::connect(
        socket_,
        &QTcpSocket::disconnected,
        this,
        &ClientConnection::onDisconnected);

    QObject::connect(
        socket_,
        &QTcpSocket::errorOccurred,
        this,
        &ClientConnection::onSocketError);
}

QString ClientConnection::peerName() const
{
    // 带上端口号。只用 IP 的话，同一台机器上开出两个客户端就会同名，
    // 服务端回传结果时就分不清该发给谁。
    return socket_->peerAddress().toString()
        + ":"
        + QString::number(socket_->peerPort());
}

qint64 ClientConnection::sendPayload(
    const QByteArray& payload)
{
    if (socket_ == nullptr
        || socket_->state()
            != QAbstractSocket::ConnectedState)
    {
        return -1;
    }

    return socket_->write(
        FrameProtocol::pack(payload));
}

void ClientConnection::onReadyRead()
{
    receiveBuffer_.append(
        socket_->readAll());

    processFrames();
}

void ClientConnection::processFrames()
{
    while (true)
    {
        QByteArray message;

        const FrameProtocol::FrameResult result =
            FrameProtocol::takeFrame(
                receiveBuffer_,
                message);

        if (result == FrameProtocol::FrameResult::Incomplete)
        {
            return;
        }

        if (result == FrameProtocol::FrameResult::Invalid)
        {
            // 长度字段明显不对，说明这条字节流已经错位，后面再也对不齐了。
            // 断开重来，而不是清掉缓冲继续撞运气。
            emit errorOccurred(
                peerName()
                + "：收到异常长度的数据，已断开连接");

            receiveBuffer_.clear();
            socket_->disconnectFromHost();
            return;
        }

        emit messageReceived(message);
    }
}

void ClientConnection::onDisconnected()
{
    emit disconnected(peerName());

    deleteLater();
}

void ClientConnection::onSocketError()
{
    emit errorOccurred(
        peerName()
        + "："
        + socket_->errorString());
}
