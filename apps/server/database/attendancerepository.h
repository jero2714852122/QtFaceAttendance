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

    // 查某人在最近 seconds 秒内有没有记录过。found 表示"查到了"，
    // 和返回值含义不同：返回 false 是查询本身失败。
    bool hasRecordWithin(
        qint64 employeeId,
        int seconds,
        bool& found);

    QString lastError() const;

private:
    DatabaseManager& databaseManager_;
    QString lastError_;
};
