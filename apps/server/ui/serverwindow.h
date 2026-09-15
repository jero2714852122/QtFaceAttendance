#pragma once

#include <QMainWindow>

class QLabel;
class QString;

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

private:
    QLabel* statusLabel_ = nullptr;
};