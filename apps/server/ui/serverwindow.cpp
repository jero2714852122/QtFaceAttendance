#include "serverwindow.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QMouseEvent>

ServerWindow::ServerWindow(
    QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "Face Attendance Server");

    setMinimumSize(
        960,
        640);

    auto* centralWidget =
        new QWidget(this);

    auto* layout =
        new QVBoxLayout(
            centralWidget);

    statusView_ =
        new QPlainTextEdit(centralWidget);

    statusView_->setReadOnly(true);
    statusView_->setMaximumBlockCount(200);
    statusView_->setPlainText(
        "服务器正在启动");

    auto* formLayout =
        new QFormLayout;

    employeeNoEdit_ =
        new QLineEdit(centralWidget);

    nameEdit_ =
        new QLineEdit(centralWidget);

    departmentEdit_ =
        new QLineEdit(centralWidget);

    formLayout->addRow(
        "员工编号：",
        employeeNoEdit_);

    formLayout->addRow(
        "姓名：",
        nameEdit_);

    formLayout->addRow(
        "部门：",
        departmentEdit_);

    layout->addLayout(
        formLayout);

    auto* buttonLayout =
        new QHBoxLayout;

    addButton_ =
        new QPushButton(
            "新增员工",
            centralWidget);

    refreshButton_ =
        new QPushButton(
            "刷新员工列表",
            centralWidget);

    deleteButton_ =
        new QPushButton(
            "删除员工",
            centralWidget);
    deleteButton_->setEnabled(false);

    QObject::connect(
        employeeList_,
        &QListWidget::itemSelectionChanged,
        this,
        [this]() {
            deleteButton_->setEnabled(
                employeeList_->currentItem() != nullptr);
        });

    buttonLayout->addWidget(
        addButton_);

    buttonLayout->addWidget(
        refreshButton_);

    buttonLayout->addWidget(
        deleteButton_);

    layout->addLayout(
        buttonLayout);

    employeeCountLabel_ =
        new QLabel(
            "当前员工：0 人",
            centralWidget);

    layout->addWidget(
        employeeCountLabel_);

    employeeList_ =
        new QListWidget(centralWidget);

    employeeList_->setAlternatingRowColors(true);

    layout->addWidget(
        employeeList_,
        1);

    statusView_->setMinimumHeight(140);

    layout->addWidget(
        statusView_);

    setCentralWidget(
        centralWidget);

    QObject::connect(
        addButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            emit addEmployeeRequested(
                employeeNoEdit_->text(),
                nameEdit_->text(),
                departmentEdit_->text());
        });

    QObject::connect(
        deleteButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            QListWidgetItem* current =
                employeeList_->currentItem();

            if (!current)
            {
                appendStatusText(
                    "请先在列表中选择一名员工");

                return;
            }

            const qint64 id =
                current->data(Qt::UserRole)
                    .toLongLong();

            const QMessageBox::StandardButton answer =
                QMessageBox::question(
                    this,
                    "删除确认",
                    "确定删除："
                        + current->text()
                        + "？该员工的考勤记录会一并清除。",
                    QMessageBox::Yes
                        | QMessageBox::No,
                    QMessageBox::No);

            if (answer != QMessageBox::Yes)
            {
                return;
            }

            emit deleteEmployeeRequested(id);
        });

    QObject::connect(
        refreshButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            emit refreshEmployeesRequested();
        });
    employeeList_->viewport()->installEventFilter(this);
}

void ServerWindow::setStatusText(
    const QString& text)
{
    statusView_->setPlainText(text);
}

void ServerWindow::appendStatusText(
    const QString& text)
{
    statusView_->appendPlainText(text);
}

void ServerWindow::setEmployeeCount(
    int count)
{
    employeeCountLabel_->setText(
        QString("当前员工：%1 人")
            .arg(count));
}
bool ServerWindow::eventFilter(
    QObject* watched,
    QEvent* event)
{
    if (watched == employeeList_->viewport()
        && event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent =
            static_cast<QMouseEvent*>(event);

        if (!employeeList_->itemAt(
                mouseEvent->position().toPoint()))
        {
            employeeList_->clearSelection();
            employeeList_->setCurrentRow(-1);
        }
    }

    return QMainWindow::eventFilter(
        watched,
        event);
}
void ServerWindow::setEmployees(
    const QList<Employee>& employees)
{
    employeeList_->clear();

    for (const Employee& employee : employees)
    {
        auto* item =
            new QListWidgetItem(
                QString("%1 | %2 | %3 | %4")
                    .arg(employee.id)
                    .arg(employee.employeeNo)
                    .arg(employee.name)
                    .arg(employee.department));

        item->setData(
            Qt::UserRole,
            employee.id);

        employeeList_->addItem(item);
    }
}
