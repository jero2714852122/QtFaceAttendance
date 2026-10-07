#pragma once

#include "facetemplate.h"

#include <QList>
#include <QString>

class DatabaseManager;

class FaceTemplateRepository
{
public:
    explicit FaceTemplateRepository(
        DatabaseManager& databaseManager);

    // 每个员工只保留一张模板，重复登记会覆盖旧的。
    bool saveTemplate(
        qint64 employeeId,
        const QByteArray& featureData);

    bool findAll(
        QList<FaceTemplate>& templates);

    QString lastError() const;

private:
    DatabaseManager& databaseManager_;
    QString lastError_;
};