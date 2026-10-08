#pragma once

#include <QByteArray>

class FrameProtocol
{
public:
    // 切帧的三种结果。用枚举而不是 bool，是因为"数据还没到齐"和
    // "长度字段明显不对"必须区别对待：前者继续等，后者只能断开重连。
    enum class FrameResult
    {
        Complete,
        Incomplete,
        Invalid
    };

    static QByteArray pack(const QByteArray& payload);

    // 切出一整帧写进 frame，并从 buffer 里移除。数据不够时什么都不动，
    // 保证补齐之后还能接着解析。
    static FrameResult takeFrame(
        QByteArray& buffer,
        QByteArray& frame);
};
