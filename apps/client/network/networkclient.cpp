#include "networkclient.h"

#include "frameprotocol.h"

#include <QDataStream>

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

    QObject::connect(
        &socket_,
        &QTcpSocket::readyRead,
        this,
        &NetworkClient::onReadyRead);
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

void NetworkClient::onReadyRead()
{
    receiveBuffer_.append(socket_.readAll());

    // 分帧规则和服务端一致：4 字节大端长度 + 内容。TCP 是字节流，
    // 一次 readyRead 可能只到了半条消息，也可能是两条粘在一起，
    // 所以必须缓冲到"长度够了"再切出来。
    constexpr quint32 maxPayloadSize = 10 * 1024 * 1024;

    while (true)
    {
        if (receiveBuffer_.size() < 4)
        {
            return;
        }

        QDataStream input(receiveBuffer_);
        input.setByteOrder(QDataStream::BigEndian);

        quint32 payloadSize = 0;
        input >> payloadSize;

        if (payloadSize > maxPayloadSize)
        {
            // 长度明显不对，说明流已经错位了，继续解析只会越错越远。
            receiveBuffer_.clear();
            return;
        }

        const qsizetype packetSize =
            4 + static_cast<qsizetype>(payloadSize);

        if (receiveBuffer_.size() < packetSize)
        {
            return;
        }

        const QByteArray message =
            receiveBuffer_.mid(
                4,
                static_cast<qsizetype>(payloadSize));

        receiveBuffer_.remove(0, packetSize);

        emit messageReceived(message);
    }
}
