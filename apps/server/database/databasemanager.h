#pragma once

#include <QString>
class DatabaseManager
{
public:
    explicit DatabaseManager(
        QString connectionName =
        "face_attendance_connection");

    ~DatabaseManager();

    bool open(
        const QString& databasePath);

    bool initializeSchema();

    void close();

    bool isOpen() const;

    QString lastError() const;

private:
    QString connectionName_;
    QString lastError_;
};