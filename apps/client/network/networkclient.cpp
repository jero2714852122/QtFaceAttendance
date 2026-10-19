#include "networkclient.h"
#include "logger.h"
#include "frameprotocol.h"

namespace
{
// 断线重连的退避参数：从 1 秒起，每失败一次翻一倍，最多 30 秒。
// 固定间隔的话，服务端停一小时客户端就白撞一千多次；几百个客户端
// 同时恢复时还会一起撞上来，把刚恢复的服务端压垮。
constexpr int kInitialReconnectDelayMs = 1000;
constexpr int kMaxReconnectDelayMs = 30000;
// 心跳间隔。客户端每隔这么久发一条 PING，服务端收到任何消息
// 都算"这个客户端还活着"。太短浪费流量，太长发现掉线太慢。
constexpr int kHeartbeatIntervalMs = 5000;

}

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent)
    , reconnectTimer_(this)
{

    QObject::connect(
        &socket_,
        &QTcpSocket::connected,
        this,
        [this]() {
            // 连上了就把退避清零，下次断线还从 1 秒开始。
            // 忘了这一句，跑一会儿就会永远卡在 30 秒才重连一次。
            reconnectDelayMs_ = kInitialReconnectDelayMs;

            reconnectTimer_.stop();
            heartbeatTimer_.start();
            emit connected();
            Logger::info(
                QString("已连接 %1:%2").arg(host_).arg(port_));

        });


    QObject::connect(
        &socket_,
        &QTcpSocket::disconnected,
        this,
        [this]() {
            startReconnectTimer();
            heartbeatTimer_.stop();
            emit disconnected();
            Logger::warning(
                QString("连接断开 %1:%2").arg(host_).arg(port_));

        });

    QObject::connect(
        &socket_,
        &QTcpSocket::errorOccurred,
        this,
        [this](QAbstractSocket::SocketError) {
            startReconnectTimer();
            heartbeatTimer_.stop();
            emit connectionError(socket_.errorString());
            Logger::error(
                QString("连接失败 %1:%2：%3")
                    .arg(host_)
                    .arg(port_)
                    .arg(socket_.errorString()));

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

    heartbeatTimer_.setInterval(kHeartbeatIntervalMs);
    QObject::connect(
    &heartbeatTimer_,
    &QTimer::timeout,
    this,
    &NetworkClient::onHeartbeatTimeout);

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

    reconnectTimer_.setInterval(reconnectDelayMs_);
    reconnectTimer_.start();

    // 先排下一次的等待时长，翻一倍但不封顶超过 30 秒。
    // 放在 start 之后，是因为这一次用的还是当前值。
    reconnectDelayMs_ = qMin(
        reconnectDelayMs_ * 2,
        kMaxReconnectDelayMs);
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
void NetworkClient::onHeartbeatTimeout()
{
    if (!isConnected())
    {
        return;
    }

    // 心跳就是一条普通的文本消息。服务端那边收到任何消息都算活着，
    // 所以不用为它发明新协议，也不会影响 JPEG 帧的处理。
    sendPayload("PING\n");
}
