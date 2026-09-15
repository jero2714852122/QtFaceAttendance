#include "clientconnection.h"

#include <QDataStream>
#include <QIODevice>
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
    return socket_->peerAddress().toString();
}

void ClientConnection::onReadyRead()
{
    receiveBuffer_.append(
        socket_->readAll());

    processFrames();
}

void ClientConnection::processFrames()
{
    constexpr quint32 maxPayloadSize =
        10 * 1024 * 1024;

    while (true)
    {
        if (receiveBuffer_.size() < 4)
        {
            return;
        }

        QDataStream input(receiveBuffer_);
        input.setByteOrder(
            QDataStream::BigEndian);

        quint32 payloadSize = 0;
        input >> payloadSize;

        if (payloadSize > maxPayloadSize)
        {
            emit errorOccurred(
                "收到消息过大，已拒绝");

            receiveBuffer_.clear();
            return;
        }

        const qsizetype packetSize =
            4 + static_cast<qsizetype>(
                payloadSize);

        if (receiveBuffer_.size() < packetSize)
        {
            return;
        }

        QByteArray message =
            receiveBuffer_.mid(
                4,
                static_cast<qsizetype>(
                    payloadSize));

        receiveBuffer_.remove(
            0,
            packetSize);

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