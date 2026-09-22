#pragma once

#include "employee.h"

#include <QList>
#include <QString>

class DatabaseManager;

class EmployeeRepository
{
public:
    explicit EmployeeRepository(
        DatabaseManager& databaseManager);

    bool addEmployee(
        const QString& employeeNo,
        const QString& name,
        const QString& department);

    bool removeById(
        qint64 id);

    bool findByEmployeeNo(
        const QString& employeeNo,
        Employee& employee);

    bool findAll(
        QList<Employee>& employees);

    QString lastError() const;

private:
    DatabaseManager& databaseManager_;
    QString lastError_;
};