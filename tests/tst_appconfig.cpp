#include <QtTest>

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>

#include "appconfig.h"

namespace
{
// 把一段 INI 内容写进临时目录，返回文件路径。
//
// 测试验的是"怎么读配置"，不该去碰仓库里那份真的 config.ini，
// 所以每个用例都在自己的临时目录里造一份。
QString writeConfig(
    const QTemporaryDir& directory,
    const QString& content)
{
    const QString path =
        directory.filePath(QStringLiteral("config.ini"));

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return QString();
    }

    file.write(content.toUtf8());
    file.close();

    return path;
}
} // namespace

class AppConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void readsEveryValidValue();
    void fallsBackWhenTheFileIsMissing();
    void fallsBackOnAnOutOfRangePort();
    void fallsBackOnANonNumericPort();
    void fallsBackOnABadListenAddress();
    void fallsBackOnAnEmptyClientAddress();
    void keepsDefaultsForMissingKeys();
};

void AppConfigTest::readsEveryValidValue()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[client]\n"
            "server_address=192.168.1.23\n"
            "server_port=51000\n"
            "[server]\n"
            "listen_address=0.0.0.0\n"
            "listen_port=50000\n"));

    const AppConfig config = AppConfig::load(path);

    QCOMPARE(
        config.server().listenAddress.toString(),
        QStringLiteral("0.0.0.0"));
    QCOMPARE(config.server().listenPort, quint16(50000));

    QCOMPARE(
        config.client().serverAddress,
        QStringLiteral("192.168.1.23"));
    QCOMPARE(config.client().serverPort, quint16(51000));

    // 全部合法时不该有任何提示，否则状态栏会被噪音占满。
    QVERIFY2(
        config.notes().isEmpty(),
        qPrintable(config.notes().join(QStringLiteral(" / "))));
}

void AppConfigTest::fallsBackWhenTheFileIsMissing()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString missing =
        directory.filePath(QStringLiteral("nope.ini"));

    const AppConfig config = AppConfig::load(missing);

    QCOMPARE(config.server().listenPort, quint16(45454));
    QCOMPARE(
        config.server().listenAddress.toString(),
        QStringLiteral("127.0.0.1"));
    QCOMPARE(
        config.client().serverAddress,
        QStringLiteral("127.0.0.1"));
    QCOMPARE(config.client().serverPort, quint16(45454));

    // 找不到文件必须说出来，不能悄悄用默认值。
    QVERIFY(!config.notes().isEmpty());
}

void AppConfigTest::fallsBackOnAnOutOfRangePort()
{
    QTemporaryDir directory;

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[server]\n"
            "listen_port=70000\n"
            "[client]\n"
            "server_port=0\n"));

    const AppConfig config = AppConfig::load(path);

    // 70000 装不进 quint16；0 会让客户端连到一个没意义的端口。
    QCOMPARE(config.server().listenPort, quint16(45454));
    QCOMPARE(config.client().serverPort, quint16(45454));
    QVERIFY(!config.notes().isEmpty());
}

void AppConfigTest::fallsBackOnANonNumericPort()
{
    QTemporaryDir directory;

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[server]\n"
            "listen_port=abc\n"));

    const AppConfig config = AppConfig::load(path);

    QCOMPARE(config.server().listenPort, quint16(45454));
    QVERIFY(!config.notes().isEmpty());
}

void AppConfigTest::fallsBackOnABadListenAddress()
{
    QTemporaryDir directory;

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[server]\n"
            "listen_address=999.999.999.999\n"));

    const AppConfig config = AppConfig::load(path);

    // 监听地址不接受主机名，也不接受拼错的 IP。
    QCOMPARE(
        config.server().listenAddress.toString(),
        QStringLiteral("127.0.0.1"));
    QVERIFY(!config.notes().isEmpty());
}

void AppConfigTest::fallsBackOnAnEmptyClientAddress()
{
    QTemporaryDir directory;

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[client]\n"
            "server_address=\n"));

    const AppConfig config = AppConfig::load(path);

    QCOMPARE(
        config.client().serverAddress,
        QStringLiteral("127.0.0.1"));
    QVERIFY(!config.notes().isEmpty());
}

void AppConfigTest::keepsDefaultsForMissingKeys()
{
    QTemporaryDir directory;

    const QString path = writeConfig(
        directory,
        QStringLiteral(
            "[server]\n"
            "listen_port=50000\n"));

    const AppConfig config = AppConfig::load(path);

    // 写了一半的配置：写了的那项生效，没写的走默认值，
    // 不能因为"只有一半"就整体作废。
    QCOMPARE(config.server().listenPort, quint16(50000));
    QCOMPARE(
        config.server().listenAddress.toString(),
        QStringLiteral("127.0.0.1"));
    QCOMPARE(
        config.client().serverAddress,
        QStringLiteral("127.0.0.1"));
    QCOMPARE(config.client().serverPort, quint16(45454));

    QVERIFY(!config.notes().isEmpty());
}

QTEST_GUILESS_MAIN(AppConfigTest)

#include "tst_appconfig.moc"
