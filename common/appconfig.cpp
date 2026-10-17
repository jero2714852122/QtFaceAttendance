#include "appconfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace
{
// 端口必须是 1 到 65535。
//
// 0 在 socket 里是"随便分我一个"的意思，对客户端来说等于连不上；
// 大于 65535 的值根本表示不了端口。两种都按"写错了"处理。
constexpr qlonglong kMinPort = 1;
constexpr qlonglong kMaxPort = 65535;

// 摄像头编号的范围。一台机器接十台以上的摄像头不现实，超出这个范围基本是写错了。
constexpr qlonglong kMaxCameraIndex = 9;

// 把一个配置值读成端口。返回 false 表示这个值不能用，调用方保留默认值。
//
// 先转成 qlonglong（装得下负数，范围也远大于端口），再自己判范围。
// 不直接转 quint16：负数会先按整数转换规则绕回来，比如 -1 变成 65535，
// 绕完之后看起来是个合法端口，就分不清"写错了"还是"真写了 65535"。
bool readPort(const QVariant& value, quint16* port)
{
    if (!value.isValid())
    {
        return false;
    }

    bool converted = false;
    const qlonglong raw = value.toLongLong(&converted);

    if (!converted || raw < kMinPort || raw > kMaxPort)
    {
        return false;
    }

    *port = static_cast<quint16>(raw);

    return true;
}
} // namespace

AppConfig AppConfig::load(const QString& path)
{
    AppConfig config;

    const QString filePath =
        path.isEmpty()
            ? QCoreApplication::applicationDirPath()
                  + QStringLiteral("/config.ini")
            : path;

    // 文件不见了不算崩溃：默认值已经写在成员初始化里，直接返回就等于全用默认值。
    // 但必须留一条说明，否则用的人会以为读成功了。
    if (!QFileInfo::exists(filePath))
    {
        config.notes_.append(
            QStringLiteral("没有找到配置文件 %1，全部使用默认值")
                .arg(QDir::toNativeSeparators(filePath)));

        return config;
    }

    QSettings settings(filePath, QSettings::IniFormat);

    // 服务端：监听地址。
    settings.beginGroup(QStringLiteral("server"));

    {
        const QVariant raw =
            settings.value(QStringLiteral("listen_address"));

        QHostAddress address;

        if (raw.isValid() && address.setAddress(raw.toString().trimmed()))
        {
            config.server_.listenAddress = address;
        }
        else
        {
            config.notes_.append(
                QStringLiteral("server.listen_address=\"%1\" 不可用，沿用默认 %2")
                    .arg(raw.toString(),
                         config.server_.listenAddress.toString()));
        }
    }

    // 服务端：监听端口。
    {
        quint16 port = 0;

        if (readPort(settings.value(QStringLiteral("listen_port")), &port))
        {
            config.server_.listenPort = port;
        }
        else
        {
            config.notes_.append(
                QStringLiteral("server.listen_port 不可用，沿用默认 %1")
                    .arg(config.server_.listenPort));
        }
    }

    settings.endGroup();

    settings.beginGroup(QStringLiteral("client"));

    // 客户端：服务器地址。
    //
    // 只查"是不是空的"，不查"是不是合法 IP"：QTcpSocket 本来就支持写主机名，
    // 写错了连接时会报错，比在这里猜更可靠。
    {
        const QString text =
            settings.value(QStringLiteral("server_address"))
                .toString()
                .trimmed();

        if (!text.isEmpty())
        {
            config.client_.serverAddress = text;
        }
        else
        {
            config.notes_.append(
                QStringLiteral("client.server_address 为空或缺失，沿用默认 %1")
                    .arg(config.client_.serverAddress));
        }
    }

    // 客户端：服务器端口。
    {
        quint16 port = 0;

        if (readPort(settings.value(QStringLiteral("server_port")), &port))
        {
            config.client_.serverPort = port;
        }
        else
        {
            config.notes_.append(
                QStringLiteral("client.server_port 不可用，沿用默认 %1")
                    .arg(config.client_.serverPort));
        }
    }

    // 客户端：摄像头编号。
    //
    // 这一项是后加的配置项，别人的 config.ini 里没有它很正常，所以"缺失"时保持
    // 沉默，直接沿用默认值；只有"写了但写错"才记一条，免得每次启动都刷一句噪音。
    {
        const QVariant raw =
            settings.value(QStringLiteral("camera_index"));

        if (raw.isValid())
        {
            bool converted = false;
            const qlonglong index = raw.toLongLong(&converted);

            if (converted && index >= 0 && index <= kMaxCameraIndex)
            {
                config.client_.cameraIndex = static_cast<int>(index);
            }
            else
            {
                config.notes_.append(
                    QStringLiteral("client.camera_index=\"%1\" 不可用，沿用默认 %2")
                        .arg(raw.toString())
                        .arg(config.client_.cameraIndex));
            }
        }
    }

    settings.endGroup();

    return config;
}
