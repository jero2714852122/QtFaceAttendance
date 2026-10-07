#pragma once

#include <QByteArray>
#include <QString>

struct FaceTemplate
{
    qint64 employeeId = 0;
    QString employeeNo;
    QString name;
    QByteArray featureData;
};