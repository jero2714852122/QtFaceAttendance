#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    QLabel* cameraPreview() const;
    QLabel* statusLabel() const;
    QLabel* identityLabel() const;
    QPushButton* startButton() const;
    QPushButton* stopButton() const;

signals:

    void startCameraRequested();
    void stopCameraRequested();

private:
    QLabel* cameraPreview_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* identityLabel_ = nullptr;
    QPushButton* startButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
};

#endif // MAINWINDOW_H