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
    updateButton_ =
        new QPushButton(
            "保存修改",
            centralWidget);
    updateButton_->setEnabled(false);
    deleteButton_->setEnabled(false);

    registerButton_ =
        new QPushButton(
            "登记人脸",
            centralWidget);

    registerButton_->setEnabled(false);

    // 登记用的是"服务端最近收到的那一帧"，这个行为不写在界面上没人猜得到。
    registerButton_->setToolTip(
        "把最近收到的一帧里的人脸登记给列表中选中的员工");

    buttonLayout->addWidget(
        addButton_);

    buttonLayout->addWidget(
        refreshButton_);

    buttonLayout->addWidget(
        deleteButton_);

    buttonLayout->addWidget(
        updateButton_);

    buttonLayout->addWidget(
        registerButton_);

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
    QObject::connect(
        employeeList_,
        &QListWidget::itemSelectionChanged,
        this,
        [this]() {
            QListWidgetItem* current =
                employeeList_->currentItem();

            const bool hasSelection =
                current != nullptr;

            deleteButton_->setEnabled(hasSelection);
            updateButton_->setEnabled(hasSelection);
            registerButton_->setEnabled(hasSelection);

            // 选中员工后工号只读：工号是业务主键，不允许修改。
            employeeNoEdit_->setReadOnly(hasSelection);

            if (!hasSelection)
            {
                employeeNoEdit_->clear();
                nameEdit_->clear();
                departmentEdit_->clear();

                return;
            }

            const Employee employee =
                current->data(Qt::UserRole)
                    .value<Employee>();

            employeeNoEdit_->setText(employee.employeeNo);
            nameEdit_->setText(employee.name);
            departmentEdit_->setText(employee.department);
        });
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

            const Employee employee =
                current->data(Qt::UserRole)
                    .value<Employee>();

            const qint64 id = employee.id;

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
        updateButton_,
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

            const Employee employee =
                current->data(Qt::UserRole)
                    .value<Employee>();

            emit updateEmployeeRequested(
                employee.id,
                nameEdit_->text(),
                departmentEdit_->text());
        });

    QObject::connect(
        registerButton_,
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

            const Employee employee =
                current->data(Qt::UserRole)
                    .value<Employee>();

            emit registerFaceRequested(employee.id);
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
            QVariant::fromValue(employee));

        employeeList_->addItem(item);
    }
}
