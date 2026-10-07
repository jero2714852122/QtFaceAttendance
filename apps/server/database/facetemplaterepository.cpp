#include "facetemplaterepository.h"

#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

FaceTemplateRepository::FaceTemplateRepository(
    DatabaseManager& databaseManager)
    : databaseManager_(databaseManager)
{
}

bool FaceTemplateRepository::saveTemplate(
    qint64 employeeId,
    const QByteArray& featureData)
{
    lastError_.clear();

    if (featureData.isEmpty())
    {
        lastError_ = "人脸特征数据为空";

        return false;
    }

    QSqlDatabase database =
        databaseManager_.database();

    // 覆盖式写入：先删旧模板再插新的。两步必须同属一个事务，
    // 否则中途失败会留下"旧模板删了、新模板没写进去"的空档，
    // 那个员工既登记不上也认不出来。
    if (!database.transaction())
    {
        lastError_ = database.lastError().text();

        return false;
    }

    QSqlQuery query(database);

    query.prepare(
        "DELETE FROM face_templates "
        "WHERE employee_id = :employee_id");

    query.bindValue(
        ":employee_id",
        employeeId);

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        database.rollback();

        return false;
    }

    query.prepare(
        "INSERT INTO face_templates "
        "(employee_id, feature_data) "
        "VALUES (:employee_id, :feature_data)");

    query.bindValue(
        ":employee_id",
        employeeId);

    query.bindValue(
        ":feature_data",
        featureData);

    if (!query.exec())
    {
        lastError_ = query.lastError().text();

        database.rollback();

        return false;
    }

    if (!database.commit())
    {
        lastError_ = database.lastError().text();

        return false;
    }

    return true;
}

bool FaceTemplateRepository::findAll(
    QList<FaceTemplate>& templates)
{
    lastError_.clear();
    templates.clear();

    QSqlQuery query(
        databaseManager_.database());

    // 用 JOIN 一次性把工号和姓名也取出来。识别成功后要显示姓名，
    // 这样就不用拿着 employee_id 回员工表再查一遍。
    if (!query.exec(
            "SELECT t.employee_id, "
            "e.employee_no, e.name, t.feature_data "
            "FROM face_templates t "
            "JOIN employees e ON e.id = t.employee_id "
            "ORDER BY t.id"))
    {
        lastError_ = query.lastError().text();

        return false;
    }

    while (query.next())
    {
        FaceTemplate faceTemplate;

        faceTemplate.employeeId =
            query.value("employee_id").toLongLong();

        faceTemplate.employeeNo =
            query.value("employee_no").toString();

        faceTemplate.name =
            query.value("name").toString();

        faceTemplate.featureData =
            query.value("feature_data").toByteArray();

        templates.append(faceTemplate);
    }

    return true;
}

QString FaceTemplateRepository::lastError() const
{
    return lastError_;
}