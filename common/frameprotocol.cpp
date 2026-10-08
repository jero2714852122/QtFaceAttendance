#include "frameprotocol.h"

#include <QDataStream>
#include <QIODevice>

namespace
{
// 单条消息的上限。超过它只可能是长度字段被读错位了，不可能是正常数据。
constexpr quint32 kMaxPayloadSize = 10 * 1024 * 1024;
}

QByteArray FrameProtocol::pack(const QByteArray& payload)
{
    QByteArray packet;

    QDataStream output(&packet, QIODevice::WriteOnly);
    output.setByteOrder(QDataStream::BigEndian);

    output << quint32(payload.size());
    packet.append(payload);

    return packet;
}

FrameProtocol::FrameResult FrameProtocol::takeFrame(
    QByteArray& buffer,
    QByteArray& frame)
{
    frame.clear();

    if (buffer.size() < 4)
    {
        return FrameResult::Incomplete;
    }

    QDataStream input(buffer);
    input.setByteOrder(QDataStream::BigEndian);

    quint32 payloadSize = 0;
    input >> payloadSize;

    if (payloadSize > kMaxPayloadSize)
    {
        return FrameResult::Invalid;
    }

    const qsizetype packetSize =
        4 + static_cast<qsizetype>(payloadSize);

    if (buffer.size() < packetSize)
    {
        return FrameResult::Incomplete;
    }

    frame = buffer.mid(
        4,
        static_cast<qsizetype>(payloadSize));

    buffer.remove(0, packetSize);

    return FrameResult::Complete;
}
