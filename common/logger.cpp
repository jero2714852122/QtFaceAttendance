#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

namespace
{
QFile logFile;
QMutex logMutex;
QString logPath;

const char* levelName(Logger::Level level)
{
    switch (level)
    {
    case Logger::Level::Warning:
        return "警告";

    case Logger::Level::Error:
        return "错误";

    case Logger::Level::Info:
    default:
        return "信息";
    }
}
} // namespace

bool Logger::open(const QString& filePath)
{
    QMutexLocker locker(&logMutex);

    logFile.close();

    const QFileInfo info(filePath);

    QDir().mkpath(info.absolutePath());

    logFile.setFileName(filePath);

    if (!logFile.open(
            QIODevice::WriteOnly
            | QIODevice::Append
            | QIODevice::Text))
    {
        logPath.clear();

        return false;
    }

    logPath = filePath;

    return true;
}

void Logger::close()
{
    QMutexLocker locker(&logMutex);

    logFile.close();
    logPath.clear();
}

void Logger::info(const QString& message)
{
    write(Level::Info, message);
}

void Logger::warning(const QString& message)
{
    write(Level::Warning, message);
}

void Logger::error(const QString& message)
{
    write(Level::Error, message);
}

QString Logger::filePath()
{
    QMutexLocker locker(&logMutex);

    return logPath;
}

void Logger::write(
    Level level,
    const QString& message)
{
    const QString line =
        QDateTime::currentDateTime()
            .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
        + QStringLiteral(" [")
        + QString::fromUtf8(levelName(level))
        + QStringLiteral("] ")
        + message;

    QMutexLocker locker(&logMutex);

    if (!logFile.isOpen())
    {
        return;
    }

    logFile.write(line.toUtf8());
    logFile.write("\n");

    // 每条都刷盘。正常情况下攒一批再写更高效，但演示程序最怕的是
    // "程序崩了，最后几行最关键的信息还没落盘"。这里用一点点性能换可靠。
    logFile.flush();
}
