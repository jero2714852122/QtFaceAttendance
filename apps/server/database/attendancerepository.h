#pragma once

#include <QString>

class DatabaseManager;

class AttendanceRepository
{
public:
    explicit AttendanceRepository(
        DatabaseManager& databaseManager);

    bool addRecord(
        qint64 employeeId,
        const QString& attendanceType,
        double confidence);


    // 单条语句完成"检查窗口 + 写入"，避免先查后插之间的竞态。
    bool addRecordUnlessRecent(
        qint64 employeeId,
        const QString& attendanceType,
        double confidence,
        int windowSeconds,
        bool& inserted);

    bool hasRecordWithin(
        qint64 employeeId,
        int seconds,
        bool& found);

    QString lastError() const;

private:
    DatabaseManager& databaseManager_;
    QString lastError_;
};
