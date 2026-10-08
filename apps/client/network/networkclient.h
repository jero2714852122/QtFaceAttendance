#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

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

    void messageReceived(const QByteArray& message);

private slots:
    void onReadyRead();
    void onReconnectTimeout();

private:
    void startReconnectTimer();

    QTcpSocket socket_;
    QByteArray receiveBuffer_;

    QString host_;
    quint16 port_ = 0;
    QTimer reconnectTimer_;
};
