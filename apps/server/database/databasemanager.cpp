#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

DatabaseManager::DatabaseManager(
    QString connectionName)
    : connectionName_(connectionName)
{

}

DatabaseManager::~DatabaseManager()
{
    close();

    if (QSqlDatabase::contains(connectionName_))
    {
        QSqlDatabase::removeDatabase(connectionName_);
    }
}

bool DatabaseManager::open(
    const QString& databasePath)
{

    lastError_.clear();

    QSqlDatabase database;

    if (QSqlDatabase::contains(connectionName_))
    {
        // false 表示只取得连接，不让 Qt 自动打开它。
        database = QSqlDatabase::database(
            connectionName_,
            false);
    }
    else
    {
        database = QSqlDatabase::addDatabase(
            "QSQLITE",
            connectionName_);
    }

    if (!database.isValid())
    {
        lastError_ = "SQLite 数据库连接无效";
        return false;
    }

    if (database.isOpen())
    {
        if (database.databaseName() == databasePath)
        {
            return true;
        }

        lastError_ =
            "数据库连接已经打开了其他文件："
            + database.databaseName();

        return false;
    }

    database.setDatabaseName(databasePath);

    if (!database.open())
    {
        lastError_ =
            database.lastError().text();

        return false;
    }

    return true;
}

bool DatabaseManager::initializeSchema()
{
    lastError_.clear();

    if (!isOpen())
    {
        lastError_ =
            "数据库尚未打开";

        return false;
    }

    QSqlDatabase database =
        QSqlDatabase::database(
            connectionName_);

    QSqlQuery query(database);

    if (!query.exec(
            "PRAGMA foreign_keys = ON"))
    {
        lastError_ =
            query.lastError().text();

        return false;
    }

    const QStringList statements = {
        R"(
            CREATE TABLE IF NOT EXISTS employees (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                employee_no TEXT NOT NULL UNIQUE,
                name TEXT NOT NULL,
                department TEXT,
                created_at TEXT NOT NULL
                    DEFAULT CURRENT_TIMESTAMP
            )
        )",

        R"(
            CREATE TABLE IF NOT EXISTS face_templates (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                employee_id INTEGER NOT NULL,
                feature_data BLOB NOT NULL,
                created_at TEXT NOT NULL
                    DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (employee_id)
                    REFERENCES employees(id)
                    ON DELETE CASCADE
            )
        )",

        R"(
            CREATE TABLE IF NOT EXISTS attendance_records (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                employee_id INTEGER NOT NULL,
                attendance_type TEXT NOT NULL,
                confidence REAL,
                created_at TEXT NOT NULL
                    DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (employee_id)
                    REFERENCES employees(id)
                    ON DELETE CASCADE
            )
        )"
    };

    for (const QString& statement : statements)
    {
        if (!query.exec(statement))
        {
            lastError_ =
                query.lastError().text();

            return false;
        }
    }

    return true;
}

void DatabaseManager::close()
{
    if (!QSqlDatabase::contains(connectionName_))
    {
        return;
    }

    QSqlDatabase database =
        QSqlDatabase::database(
            connectionName_,
            false);

    if (database.isValid())
    {
        database.close();
    }
}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(connectionName_))
    {
        return false;
    }

    return QSqlDatabase::database(
               connectionName_,
               false)
        .isOpen();
}

QString DatabaseManager::lastError() const
{
    return lastError_;
}
//对原生api做个封装，防止如果不存在连接名新注册一个无效连接
QSqlDatabase DatabaseManager::database() const
{
    if (!QSqlDatabase::contains(connectionName_))
    {
        return {};
    }

    return QSqlDatabase::database(
        connectionName_,
        false);
}