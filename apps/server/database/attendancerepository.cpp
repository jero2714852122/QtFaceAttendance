#include "attendancerepository.h"

#include "databasemanager.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimeZone>
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

bool AttendanceRepository::addRecordUnlessRecent(
    qint64 employeeId,
    const QString& attendanceType,
    double confidence,
    int windowSeconds,
    bool& inserted)
{
    lastError_.clear();
    inserted = false;

    QSqlQuery query(
        databaseManager_.database());

    query.prepare(
        "INSERT INTO attendance_records "
        "(employee_id, attendance_type, confidence) "
        "SELECT :employee_id, :attendance_type, :confidence "
        "WHERE NOT EXISTS ("
        "    SELECT 1 FROM attendance_records "
        "    WHERE employee_id = :employee_id "
        "      AND created_at > datetime('now', :offset)"
        ")");

    query.bindValue(":employee_id", employeeId);
    query.bindValue(":attendance_type", attendanceType);
    query.bindValue(":confidence", confidence);

    query.bindValue(
        ":offset",
        QString("-%1 seconds").arg(windowSeconds));

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        return false;
    }

    // 插进去了是 1；被 WHERE NOT EXISTS 挡掉是 0。
    inserted = query.numRowsAffected() > 0;

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

bool AttendanceRepository::findRecords(
    const QDateTime& fromLocal,
    const QDateTime& toLocal,
    const QString& keyword,
    QList<AttendanceRow>& rows)
{
    lastError_.clear();
    rows.clear();

    // 界面给的是本地时间，库里存的是 UTC。换算只在这一个地方做。
    const QString fromText =
        fromLocal.toUTC().toString(
            QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    const QString toText =
        toLocal.toUTC().toString(
            QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    const QString cleanKeyword = keyword.trimmed();
    const bool filterByKeyword = !cleanKeyword.isEmpty();

    // LIKE 里的 % 和 _ 是通配符。用户搜 "50%" 时如果直接拼进去，
    // 会变成"以 50 开头"这种意思。所以要先转义，再用 ESCAPE 声明转义符。
    QString likePattern;

    if (filterByKeyword)
    {
        QString escaped = cleanKeyword;

        escaped.replace("\\", "\\\\");
        escaped.replace("%", "\\%");
        escaped.replace("_", "\\_");

        likePattern = "%" + escaped + "%";
    }

    QSqlQuery query(
        databaseManager_.database());

    // :no_filter 是个开关。没关键字时绑 1，整个括号恒真，三个 LIKE 都不起作用。
    // 这样只用一条 SQL，不用维护"带筛选"和"不带筛选"两个版本。
    query.prepare(
        "SELECT a.created_at, e.employee_no, e.name, "
        "       e.department, a.attendance_type, a.confidence "
        "FROM attendance_records a "
        "JOIN employees e ON e.id = a.employee_id "
        "WHERE a.created_at >= :from AND a.created_at < :to "
        "  AND (:no_filter = 1 "
        "       OR e.employee_no LIKE :like ESCAPE '\\' "
        "       OR e.name LIKE :like ESCAPE '\\' "
        "       OR e.department LIKE :like ESCAPE '\\') "
        "ORDER BY a.created_at DESC");

    query.bindValue(":from", fromText);
    query.bindValue(":to", toText);
    query.bindValue(":no_filter", filterByKeyword ? 0 : 1);
    query.bindValue(":like", likePattern);

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        return false;
    }

    while (query.next())
    {
        // 库里那串字符是 UTC，但 fromString 解析出来的对象不带时区，
        // Qt 会当成本地时间。所以要先标成 UTC，再转本地，否则差八小时。
        QDateTime createdAt = QDateTime::fromString(
            query.value(0).toString(),
            QStringLiteral("yyyy-MM-dd HH:mm:ss"));

        createdAt.setTimeZone(QTimeZone::UTC);

        AttendanceRow row;

        row.createdAt =
            createdAt.toLocalTime().toString(
                QStringLiteral("yyyy-MM-dd HH:mm:ss"));

        row.employeeNo = query.value(1).toString();
        row.name = query.value(2).toString();
        row.department = query.value(3).toString();
        row.attendanceType = query.value(4).toString();
        row.confidence = query.value(5).toDouble();

        rows.append(row);
    }

    return true;
}

QString AttendanceRepository::lastError() const
{
    return lastError_;
}
