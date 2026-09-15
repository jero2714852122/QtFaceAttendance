#include "serverwindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

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

    statusLabel_ =
        new QLabel(
            "服务器正在启动",
            centralWidget);

    statusLabel_->setAlignment(
        Qt::AlignCenter);

    layout->addWidget(
        statusLabel_);

    setCentralWidget(
        centralWidget);
}

void ServerWindow::setStatusText(
    const QString& text)
{
    statusLabel_->setText(text);
}

void ServerWindow::appendStatusText(
    const QString& text)
{
    statusLabel_->setText(
        statusLabel_->text()
        + "\n"
        + text);
}