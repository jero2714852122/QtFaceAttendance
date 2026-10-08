#include "networkclient.h"

#include "frameprotocol.h"

namespace
{
// 断线后每隔几秒重试一次。演示时经常是先开客户端再开服务端，
// 没有自动重连的话，每次都得手动把客户端重启一遍。
constexpr int kReconnectIntervalMs = 3000;
}

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent)
    , reconnectTimer_(this)
{
    reconnectTimer_.setInterval(kReconnectIntervalMs);

    QObject::connect(
        &socket_,
        &QTcpSocket::connected,
        this,
        [this]() {
            reconnectTimer_.stop();
            emit connected();
        });

    QObject::connect(
        &socket_,
        &QTcpSocket::disconnected,
        this,
        [this]() {
            startReconnectTimer();
            emit disconnected();
        });

    QObject::connect(
        &socket_,
        &QTcpSocket::errorOccurred,
        this,
        [this](QAbstractSocket::SocketError) {
            startReconnectTimer();
            emit connectionError(socket_.errorString());
        });

    QObject::connect(
        &socket_,
        &QTcpSocket::readyRead,
        this,
        &NetworkClient::onReadyRead);

    QObject::connect(
        &reconnectTimer_,
        &QTimer::timeout,
        this,
        &NetworkClient::onReconnectTimeout);
}

void NetworkClient::connectToServer(const QString& host, quint16 port)
{
    // 记住地址是为了断线之后能自己接回来。
    host_ = host;
    port_ = port;

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
            // 服务端不可能发出长度非法的消息。真出现了说明这条连接
            // 已经不可信，断开它，交给自动重连接回来。
            receiveBuffer_.clear();
            socket_.abort();
            return;
        }

        emit messageReceived(message);
    }
}

void NetworkClient::startReconnectTimer()
{
    // 没配过地址说明还没发起过连接，没什么可重试的。
    if (host_.isEmpty())
    {
        return;
    }

    reconnectTimer_.start();
}

void NetworkClient::onReconnectTimeout()
{
    if (isConnected()
        || socket_.state()
            == QAbstractSocket::ConnectingState)
    {
        return;
    }

    // 先把上一次失败的连接彻底丢掉再重连。半死不活的 socket 直接
    // 调用 connectToHost 有时会卡在旧状态里连不上。
    socket_.abort();
    socket_.connectToHost(host_, port_);
}
