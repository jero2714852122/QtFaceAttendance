#include "attendancerepository.h"

#include "databasemanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

AttendanceRepository::AttendanceRepository(
    DatabaseManager& databaseManager)
    : databaseManager_(databaseManager)
{
}

bool AttendanceRepository::addRecord(
    qint64 employeeId,
    const QString& attendanceType,
    double confidence)
{
    lastError_.clear();

    QSqlQuery query(
        databaseManager_.database());

    query.prepare(
        "INSERT INTO attendance_records "
        "(employee_id, attendance_type, confidence) "
        "VALUES (:employee_id, :attendance_type, :confidence)");

    query.bindValue(":employee_id", employeeId);
    query.bindValue(":attendance_type", attendanceType);
    query.bindValue(":confidence", confidence);

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        return false;
    }

    return true;
}

bool AttendanceRepository::hasRecordWithin(
    qint64 employeeId,
    int seconds,
    bool& found)
{
    lastError_.clear();
    found = false;

    QSqlQuery query(
        databaseManager_.database());

    // created_at 由 CURRENT_TIMESTAMP 写入，值是 UTC；datetime('now') 也是
    // UTC，两边同一时区才能直接比字符串。这里如果用 Qt 的本地时间去比，
    // 在 +8 时区会差 8 小时，限流窗口要么永远命中、要么永远不命中。
    query.prepare(
        "SELECT COUNT(*) AS total "
        "FROM attendance_records "
        "WHERE employee_id = :employee_id "
        "AND created_at > datetime('now', :offset)");

    query.bindValue(":employee_id", employeeId);

    query.bindValue(
        ":offset",
        QString("-%1 seconds").arg(seconds));

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        return false;
    }

    if (!query.next())
    {
        return true;
    }

    found = query.value("total").toInt() > 0;

    return true;
}

QString AttendanceRepository::lastError() const
{
    return lastError_;
}
