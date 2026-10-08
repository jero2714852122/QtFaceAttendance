#include <QtTest>

#include <QSqlQuery>
#include <QTemporaryDir>

#include <memory>

#include "database/attendancerepository.h"
#include "database/databasemanager.h"
#include "database/employeerepository.h"
#include "database/facetemplaterepository.h"

class DatabaseTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void createsTheSchema();
    void addsAndFindsAnEmployee();
    void rejectsADuplicateEmployeeNumber();
    void updatesOnlyTheNameAndDepartment();
    void removesAnEmployeeAndItsTemplate();
    void rateLimitsAttendance();
    void ignoresRecordsOutsideTheWindow();

private:
    QTemporaryDir directory_;
    std::unique_ptr<DatabaseManager> database_;
    int counter_ = 0;
};

void DatabaseTest::init()
{
    QVERIFY(directory_.isValid());

    // 每个用例用独立的连接名和独立的库文件。共用一个文件的话，
    // 用例之间会互相污染，失败时也说不清是谁弄脏了数据。
    ++counter_;

    database_ = std::make_unique<DatabaseManager>(
        QString("test_connection_%1").arg(counter_));

    const QString databasePath =
        directory_.filePath(
            QString("test_%1.db").arg(counter_));

    QVERIFY2(
        database_->open(databasePath),
        qPrintable(database_->lastError()));

    QVERIFY2(
        database_->initializeSchema(),
        qPrintable(database_->lastError()));
}

void DatabaseTest::cleanup()
{
    database_.reset();
}

void DatabaseTest::createsTheSchema()
{
    QSqlQuery query(database_->database());

    QVERIFY(query.exec(
        "SELECT name FROM sqlite_master "
        "WHERE type = 'table' "
        "AND name IN ('employees', 'face_templates', 'attendance_records')"));

    QStringList tables;

    while (query.next())
    {
        tables.append(query.value(0).toString());
    }

    tables.sort();

    QCOMPARE(tables, QStringList({"attendance_records", "employees", "face_templates"}));
}

void DatabaseTest::addsAndFindsAnEmployee()
{
    EmployeeRepository repository(*database_);

    QVERIFY2(
        repository.addEmployee("1001", "张三", "研发部"),
        qPrintable(repository.lastError()));

    Employee employee;

    QVERIFY2(
        repository.findByEmployeeNo("1001", employee),
        qPrintable(repository.lastError()));

    QCOMPARE(employee.name, QString("张三"));
    QCOMPARE(employee.department, QString("研发部"));

    QList<Employee> all;
    QVERIFY(repository.findAll(all));
    QCOMPARE(all.size(), 1);
}

void DatabaseTest::rejectsADuplicateEmployeeNumber()
{
    EmployeeRepository repository(*database_);

    QVERIFY(repository.addEmployee("1001", "张三", "研发部"));

    // 唯一约束是最后一道防线。前面加过查重也不能把它去掉：
    // 查重和写入之间有间隙，两个进程同时插入时只有约束拦得住。
    QVERIFY(!repository.addEmployee("1001", "李四", "市场部"));
    QVERIFY(!repository.lastError().isEmpty());

    QList<Employee> all;
    QVERIFY(repository.findAll(all));
    QCOMPARE(all.size(), 1);
}

void DatabaseTest::updatesOnlyTheNameAndDepartment()
{
    EmployeeRepository repository(*database_);

    QVERIFY(repository.addEmployee("1001", "张三", "研发部"));

    Employee employee;
    QVERIFY(repository.findByEmployeeNo("1001", employee));

    QVERIFY2(
        repository.updateEmployee(
            employee.id,
            "张三丰",
            "技术部"),
        qPrintable(repository.lastError()));

    Employee updated;
    QVERIFY(repository.findByEmployeeNo("1001", updated));

    QCOMPARE(updated.name, QString("张三丰"));
    QCOMPARE(updated.department, QString("技术部"));
    QCOMPARE(updated.employeeNo, QString("1001"));
}

void DatabaseTest::removesAnEmployeeAndItsTemplate()
{
    EmployeeRepository employees(*database_);
    FaceTemplateRepository templates(*database_);

    QVERIFY(employees.addEmployee("1001", "张三", "研发部"));

    Employee employee;
    QVERIFY(employees.findByEmployeeNo("1001", employee));

    QVERIFY(templates.saveTemplate(
        employee.id,
        QByteArray(512, '\0')));

    QList<FaceTemplate> stored;
    QVERIFY(templates.findAll(stored));
    QCOMPARE(stored.size(), 1);

    QVERIFY2(
        employees.removeById(employee.id),
        qPrintable(employees.lastError()));

    // 外键写的是 ON DELETE CASCADE，删员工要连带删掉他的人脸模板。
    // 否则库里会留下指向不存在员工的孤儿数据，识别时还会拿它去比对。
    QVERIFY(templates.findAll(stored));
    QCOMPARE(stored.size(), 0);
}

void DatabaseTest::rateLimitsAttendance()
{
    EmployeeRepository employees(*database_);
    AttendanceRepository attendance(*database_);

    QVERIFY(employees.addEmployee("1001", "张三", "研发部"));

    Employee employee;
    QVERIFY(employees.findByEmployeeNo("1001", employee));

    bool found = true;

    // 还没记过，窗口内应该是空的。
    QVERIFY2(
        attendance.hasRecordWithin(employee.id, 120, found),
        qPrintable(attendance.lastError()));
    QVERIFY(!found);

    QVERIFY2(
        attendance.addRecord(employee.id, "check_in", 0.87),
        qPrintable(attendance.lastError()));

    // 刚记完，同一个窗口内就该查得到，这样限流才拦得住。
    QVERIFY(attendance.hasRecordWithin(employee.id, 120, found));
    QVERIFY(found);

    // 别人不该受影响：限流是按人算的，不是全局的。
    QVERIFY(employees.addEmployee("1002", "李四", "市场部"));

    Employee other;
    QVERIFY(employees.findByEmployeeNo("1002", other));

    QVERIFY(attendance.hasRecordWithin(other.id, 120, found));
    QVERIFY(!found);
}

void DatabaseTest::ignoresRecordsOutsideTheWindow()
{
    EmployeeRepository employees(*database_);
    AttendanceRepository attendance(*database_);

    QVERIFY(employees.addEmployee("1003", "王五", "财务部"));

    Employee employee;
    QVERIFY(employees.findByEmployeeNo("1003", employee));

    // 手工插一条两小时前的记录。窗口判断必须是"时间范围内的记录"，
    // 不能简化成"这个人有没有记录过"，否则限流就变成了永久生效。
    QSqlQuery insert(database_->database());

    insert.prepare(
        "INSERT INTO attendance_records "
        "(employee_id, attendance_type, confidence, created_at) "
        "VALUES (:employee_id, 'check_in', 0.9, datetime('now', '-2 hours'))");

    insert.bindValue(":employee_id", employee.id);

    QVERIFY(insert.exec());

    bool found = true;

    QVERIFY(attendance.hasRecordWithin(employee.id, 120, found));
    QVERIFY(!found);
}

// 用 GUILESS 而不是 APPLESS：SQL 模块必须要有一个 QCoreApplication
// 实例才能加载驱动，APPLESS 不创建它，加数据库会直接崩。
QTEST_GUILESS_MAIN(DatabaseTest)

#include "tst_database.moc"
