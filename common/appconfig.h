#pragma once

#include <QHostAddress>
#include <QString>
#include <QStringList>

// 应用的运行配置。
//
// 这个类只干一件事：把 config.ini 里的字符串，变成程序能直接用的一组值。
// 它不负责"怎么用这些值"，那是 NetworkClient 和 Server 的事。
class AppConfig
{
public:
    // 服务端要的东西：在哪个地址、哪个端口等人来连。
    struct ServerSettings
    {
        QHostAddress listenAddress
            = QHostAddress(QHostAddress::LocalHost);
        quint16 listenPort = 45454;
    };

    // 客户端要的东西：去连哪个地址、哪个端口，以及用哪个摄像头。
    struct ClientSettings
    {
        QString serverAddress = QStringLiteral("127.0.0.1");
        quint16 serverPort = 45454;

        // 摄像头编号。0 一般是内置摄像头，外接的 USB 摄像头通常是 1 或 2。
        // 编号不存在时不会崩，CameraController::open() 会返回失败并提示。
        int cameraIndex = 0;
    };

    // 读配置文件。
    //
    // path 留空时读可执行文件旁边的 config.ini；显式传路径主要是为了测试。
    // 任何一项缺失或不合法都不会失败，只会保留默认值，并把原因记进 notes()。
    static AppConfig load(const QString& path = QString());

    const ServerSettings& server() const { return server_; }

    const ClientSettings& client() const { return client_; }

    // 读取过程中发现的问题，一条一行。全部正常时是空的。
    QStringList notes() const { return notes_; }

private:
    ServerSettings server_;

    ClientSettings client_;

    QStringList notes_;
};
