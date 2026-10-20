#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

class DatabaseManager;

// 查出来的一行考勤。字段是给界面显示准备的，
// 所以时间已经转成本地时间。
struct AttendanceRow
{
    QString createdAt;
    QString employeeNo;
    QString name;
    QString department;
    QString attendanceType;
    double confidence = 0.0;
};

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

    // 按时间范围和关键字查考勤。
    //
    // fromLocal / toLocal 是**本地时间**。库里 created_at 存的是 UTC，
    // 换算在这里做，调用方不用操心。
    //
    // keyword 为空表示不按人筛选；否则同时匹配工号、姓名、部门。
    bool findRecords(
        const QDateTime& fromLocal,
        const QDateTime& toLocal,
        const QString& keyword,
        QList<AttendanceRow>& rows);

    QString lastError() const;

private:
    DatabaseManager& databaseManager_;
    QString lastError_;
};
