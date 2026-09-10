#pragma once

#include <QByteArray>

class FrameProtocol
{
public:
    static QByteArray pack(const QByteArray& payload);
};