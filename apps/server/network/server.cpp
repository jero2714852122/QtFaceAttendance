#include "server.h"

#include "clientconnection.h"

Server::Server(QObject* parent)
    : QObject(parent)
{
    QObject::connect(
        &server_,
        &QTcpServer::newConnection,
        this,
        &Server::onNewConnection);
}

bool Server::listen(
    const QHostAddress& address,
    quint16 port)
{
    if (server_.listen(address, port))
    {
        emit listeningStarted(
            "服务器正在监听 "
            + address.toString()
            + ":"
            + QString::number(port));

        return true;
    }

    emit listeningError(
        "服务器监听失败："
        + server_.errorString());

    return false;
}

void Server::close()
{
    server_.close();
}

void Server::onNewConnection()
{
    while (server_.hasPendingConnections())
    {
        QTcpSocket* socket =
            server_.nextPendingConnection();

        auto* connection =
            new ClientConnection(
                socket,
                this);

        connections_.append(connection);

        const QString peer =
            connection->peerName();

        emit clientConnected(peer);

        QObject::connect(
            connection,
            &ClientConnection::messageReceived,
            this,
            [this, connection](
                const QByteArray& message) {
                emit messageReceived(
                    connection->peerName(),
                    message);
            });

        QObject::connect(
            connection,
            &ClientConnection::disconnected,
            this,
            [this, connection](const QString& peer) {
                connections_.removeOne(connection);
                emit clientDisconnected(peer);
            });

        QObject::connect(
            connection,
            &ClientConnection::errorOccurred,
            this,
            &Server::clientError);
    }
}