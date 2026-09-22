#pragma once

#include <QString>

struct Employee
{
    qint64 id = 0;
    QString employeeNo;
    QString name;
    QString department;
    QString createdAt;
};