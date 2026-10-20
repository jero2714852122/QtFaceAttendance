#pragma once

#include <QDialog>

class AttendanceRepository;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

class AttendanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttendanceDialog(
        AttendanceRepository& repository,
        QWidget* parent = nullptr);

private slots:
    void onQuery();
    void onExport();

private:
    AttendanceRepository& repository_;

    QDateEdit* fromEdit_ = nullptr;
    QDateEdit* toEdit_ = nullptr;
    QLineEdit* keywordEdit_ = nullptr;
    QPushButton* queryButton_ = nullptr;
    QPushButton* exportButton_ = nullptr;
    QTableWidget* table_ = nullptr;
    QLabel* summaryLabel_ = nullptr;
};
