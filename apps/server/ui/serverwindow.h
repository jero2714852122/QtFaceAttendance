#pragma once

#include <QMainWindow>
#include<QList>
#include<QString>
#include "database/employee.h"
class QLineEdit;
class QListWidget;
class QLabel;
class QPushButton;
class QPlainTextEdit;

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(
        QWidget* parent = nullptr);

public slots:
    void setStatusText(
        const QString& text);

    void appendStatusText(
        const QString& text);

    void setEmployeeCount(
        int count);

    void setEmployees(
        const QList<Employee>& employees);
signals:
    void addEmployeeRequested(
        const QString& employeeNo,
        const QString& name,
        const QString& department);

    void refreshEmployeesRequested();
    void deleteEmployeeRequested(
        qint64 id);
protected:
    bool eventFilter(
        QObject* watched,
        QEvent* event) override;
private:
    QPlainTextEdit* statusView_ = nullptr;
    QLabel* employeeCountLabel_ = nullptr;
    QLineEdit* employeeNoEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* departmentEdit_ = nullptr;
    QListWidget* employeeList_ = nullptr;
    QPushButton* addButton_ = nullptr;
    QPushButton* refreshButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
};
