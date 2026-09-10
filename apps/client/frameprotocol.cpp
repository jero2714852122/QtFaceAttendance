#include "frameprotocol.h"

#include <QDataStream>
#include <QIODevice>

QByteArray FrameProtocol::pack(const QByteArray& payload)
{
    QByteArray packet;

    QDataStream output(&packet, QIODevice::WriteOnly);
    output.setByteOrder(QDataStream::BigEndian);

    output << quint32(payload.size());
    packet.append(payload);

    return packet;
}