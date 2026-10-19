#pragma once

#include <QString>

// 极简日志：写文件，带时间戳和级别。
//
// 为什么用静态接口而不是对象：一个进程只有一个日志流，做成全局的更直白，调用方不用到处传一个 logger 引用。
// 内部用互斥量保护，所以从工作线程里调也安全。
class Logger
{
public:
    enum class Level
    {
        Info,
        Warning,
        Error
    };

    // 打开日志文件。失败返回 false——程序照常跑，只是没有日志。
    // 日志是辅助功能，不能因为它出问题就让主程序起不来。
    static bool open(const QString& filePath);

    static void close();

    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

    static QString filePath();

private:
    static void write(
        Level level,
        const QString& message);
};
