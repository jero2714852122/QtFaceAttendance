#include "employeerepository.h"

#include "databasemanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

EmployeeRepository::EmployeeRepository(
    DatabaseManager& databaseManager)
    : databaseManager_(databaseManager)
{
}

bool EmployeeRepository::addEmployee(
    const QString& employeeNo,
    const QString& name,
    const QString& department)
{
    lastError_.clear();

    QSqlQuery query(
        databaseManager_.database());

    query.prepare(
        "INSERT INTO employees "
        "(employee_no, name, department) "
        "VALUES "
        "(:employee_no, :name, :department)");

    query.bindValue(
        ":employee_no",
        employeeNo);

    query.bindValue(
        ":name",
        name);

    query.bindValue(
        ":department",
        department);

    if (!query.exec())
    {
        lastError_ =
            query.lastError().text();

        return false;
    }

    return true;
}

bool EmployeeRepository::removeById(
    qint64 id)
{
    lastError_.clear();

    QSqlQuery query(
        databaseManager_.database());

    query.prepare(
        "DELETE FROM employees "
        "WHERE id = :id");

    query.bindValue(
        ":id",
        id);

    if (!query.exec())
    {
        lastError_ =
            query.lastError().text();

        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        lastError_ =
            "未找到该员工，删除未生效";

        return false;
    }

    return true;
}

bool EmployeeRepository::findByEmployeeNo(
    const QString& employeeNo,
    Employee& employee)
{
    lastError_.clear();

    QSqlQuery query(
        databaseManager_.database());

    query.prepare(
        "SELECT id, employee_no, name, "
        "department, created_at "
        "FROM employees "
        "WHERE employee_no = :employee_no");

    query.bindValue(
        ":employee_no",
        employeeNo);

    if (!query.exec())
    {
        lastError_ =
            query.lastError().text();

        return false;
    }

    if (!query.next())
    {
        lastError_ =
            "未找到该员工";

        return false;
    }

    employee.id =
        query.value("id").toLongLong();

    employee.employeeNo =
        query.value("employee_no").toString();

    employee.name =
        query.value("name").toString();

    employee.department =
        query.value("department").toString();

    employee.createdAt =
        query.value("created_at").toString();

    return true;
}

bool EmployeeRepository::findAll(
    QList<Employee>& employees)
{
    lastError_.clear();
    employees.clear();

    QSqlQuery query(
        databaseManager_.database());

    if (!query.exec(
            "SELECT id, employee_no, name, "
            "department, created_at "
            "FROM employees "
            "ORDER BY id"))
    {
        lastError_ =
            query.lastError().text();

        return false;
    }

    while (query.next())
    {
        Employee employee;

        employee.id =
            query.value("id").toLongLong();

        employee.employeeNo =
            query.value("employee_no").toString();

        employee.name =
            query.value("name").toString();

        employee.department =
            query.value("department").toString();

        employee.createdAt =
            query.value("created_at").toString();

        employees.append(employee);
    }

    return true;
}

QString EmployeeRepository::lastError() const
{
    return lastError_;
}