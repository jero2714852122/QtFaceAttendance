#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QImage>
#include<QString>

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    void setStatusText(const QString& text);
    void setFaceCount(int count);
    void setCameraRunning(bool running);
    void showPreviewImage(const QImage& image);
    void resetPreview();

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

#endif
