#include "attendancedialog.h"

#include "database/attendancerepository.h"
#include "logger.h"

#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTime>
#include <QVBoxLayout>

AttendanceDialog::AttendanceDialog(
    AttendanceRepository& repository,
    QWidget* parent)
    : QDialog(parent)
    , repository_(repository)
{
    setWindowTitle("考勤查询");
    setMinimumSize(880, 560);

    auto* layout = new QVBoxLayout(this);

    auto* filterLayout = new QHBoxLayout;

    fromEdit_ = new QDateEdit(QDate::currentDate(), this);
    fromEdit_->setCalendarPopup(true);
    fromEdit_->setDisplayFormat("yyyy-MM-dd");

    toEdit_ = new QDateEdit(QDate::currentDate(), this);
    toEdit_->setCalendarPopup(true);
    toEdit_->setDisplayFormat("yyyy-MM-dd");

    keywordEdit_ = new QLineEdit(this);
    keywordEdit_->setPlaceholderText("工号 / 姓名 / 部门，留空表示不筛选");

    queryButton_ = new QPushButton("查询", this);
    exportButton_ = new QPushButton("导出 CSV", this);

    filterLayout->addWidget(new QLabel("从", this));
    filterLayout->addWidget(fromEdit_);
    filterLayout->addWidget(new QLabel("到", this));
    filterLayout->addWidget(toEdit_);
    filterLayout->addWidget(keywordEdit_, 1);
    filterLayout->addWidget(queryButton_);
    filterLayout->addWidget(exportButton_);

    layout->addLayout(filterLayout);

    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels(
        {"时间", "工号", "姓名", "部门", "类型", "相似度"});
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(table_, 1);

    summaryLabel_ = new QLabel("还没查询", this);
    layout->addWidget(summaryLabel_);

    QObject::connect(
        queryButton_, &QPushButton::clicked,
        this, &AttendanceDialog::onQuery);

    QObject::connect(
        exportButton_, &QPushButton::clicked,
        this, &AttendanceDialog::onExport);
}

void AttendanceDialog::onQuery()
{
    // 用户选的是"哪一天"，本地时间。查的时候从当天 00:00 到次日 00:00，
    // 也就是"包含结束那一天"。换算成 UTC 是 findRecords 的事。
    const QDateTime fromLocal(fromEdit_->date(), QTime(0, 0));
    const QDateTime toLocal(toEdit_->date().addDays(1), QTime(0, 0));

    QList<AttendanceRow> rows;

    if (!repository_.findRecords(
            fromLocal, toLocal, keywordEdit_->text(), rows))
    {
        QMessageBox::warning(this, "查询失败", repository_.lastError());
        return;
    }

    table_->setRowCount(rows.size());

    for (int i = 0; i < rows.size(); ++i)
    {
        const AttendanceRow& row = rows.at(i);

        table_->setItem(i, 0, new QTableWidgetItem(row.createdAt));
        table_->setItem(i, 1, new QTableWidgetItem(row.employeeNo));
        table_->setItem(i, 2, new QTableWidgetItem(row.name));
        table_->setItem(i, 3, new QTableWidgetItem(row.department));
        table_->setItem(i, 4, new QTableWidgetItem(
                                  row.attendanceType == QStringLiteral("check_in")
                                      ? QStringLiteral("签到")
                                      : row.attendanceType));
        table_->setItem(i, 5, new QTableWidgetItem(
                                  QString::number(row.confidence, 'f', 4)));
    }

    summaryLabel_->setText(
        QString("共 %1 条（%2 到 %3）")
            .arg(rows.size())
            .arg(fromLocal.toString("yyyy-MM-dd"))
            .arg(toEdit_->date().toString("yyyy-MM-dd")));

    Logger::info(QString("考勤查询：%1 条").arg(rows.size()));
}

void AttendanceDialog::onExport()
{
    if (table_->rowCount() == 0)
    {
        QMessageBox::information(this, "没有数据", "先查询，再导出。");
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, "导出考勤", "attendance.csv", "CSV 文件 (*.csv)");

    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "导出失败", file.errorString());
        return;
    }

    // Excel 打开 UTF-8 的 CSV 会乱码，除非文件开头有 BOM。这三个字节就是 BOM。
    file.write("\xEF\xBB\xBF");

    file.write(QByteArray("时间,工号,姓名,部门,类型,相似度\n"));

    for (int r = 0; r < table_->rowCount(); ++r)
    {
        QStringList cells;

        for (int c = 0; c < table_->columnCount(); ++c)
        {
            QTableWidgetItem* item = table_->item(r, c);
            QString text = item ? item->text() : QString();

            // 逗号、引号、换行会破坏 CSV 的结构：用双引号包起来，
            // 内容里原有的引号要翻倍。
            if (text.contains(',')
                || text.contains('"')
                || text.contains('\n'))
            {
                text.replace("\"", "\"\"");
                text = "\"" + text + "\"";
            }

            cells.append(text);
        }

        file.write(cells.join(',').toUtf8());
        file.write("\n");
    }

    file.close();

    Logger::info("考勤已导出：" + path);
    QMessageBox::information(this, "导出完成", path);
}
