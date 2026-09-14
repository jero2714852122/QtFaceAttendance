#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTcpSocket>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject* parent = nullptr);
    void connectToServer(const QString& host, quint16 port);
    qint64 sendPayload(const QByteArray& payload);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& errorMessage);

private:
    QTcpSocket socket_;
};