#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QString>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("人脸考勤系统");
    setMinimumSize(960, 640);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    cameraPreview_ = new QLabel("相机预览", centralWidget);
    cameraPreview_->setAlignment(Qt::AlignCenter);
    cameraPreview_->setMinimumHeight(360);
    mainLayout->addWidget(cameraPreview_, 1);

    statusLabel_ = new QLabel("状态：等待摄像头开启", centralWidget);
    statusLabel_->setMinimumHeight(40);
    statusLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(statusLabel_);

    identityLabel_ = new QLabel("身份：未能识别身份", centralWidget);
    identityLabel_->setMinimumHeight(40);
    identityLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(identityLabel_);

    QHBoxLayout* actionLayout = new QHBoxLayout;

    startButton_ = new QPushButton("打开摄像头", centralWidget);
    stopButton_ = new QPushButton("关闭摄像头", centralWidget);
    stopButton_->setEnabled(false);

    actionLayout->addWidget(startButton_);
    actionLayout->addWidget(stopButton_);
    mainLayout->addLayout(actionLayout);

    setCentralWidget(centralWidget);

    QObject::connect(
        startButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            emit startCameraRequested();
        });

    QObject::connect(
        stopButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            emit stopCameraRequested();
        });
}
void MainWindow::setStatusText(const QString&text)
{
    statusLabel_->setText(text);
}
void MainWindow::setFaceCount(int count)
{
    identityLabel_->setText(QString("检测到人脸数量：%1").arg(count));
}
void MainWindow::setCameraRunning(bool running)
{
    startButton_->setEnabled(!running);
    stopButton_->setEnabled(running);
}
void MainWindow::showPreviewImage(const QImage&image)
{
    cameraPreview_->setPixmap(QPixmap::fromImage(image).scaled(cameraPreview_->size(),
        Qt::KeepAspectRatio,
        Qt::FastTransformation));
}
void MainWindow::resetPreview()
{
    cameraPreview_->clear();
    cameraPreview_->setText("相机预览");
}