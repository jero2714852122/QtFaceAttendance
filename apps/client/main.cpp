#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include<QWidget>
#include<QVBoxLayout>
#include<QPushButton>
#include<QHBoxLayout>
#include<QObject>
#include<QTimer>
#include<opencv2/videoio.hpp>
#include<QImage>
#include<QPixmap>
#include <QCoreApplication>
#include <QElapsedTimer>
#include<vector>
#include<opencv2/objdetect.hpp>
#include<QIODevice>
#include"mainwindow.h"
#include "cameracontroller.h"
#include"facedetector.h"
#include"networkclient.h"
#include "frameprocessor.h"
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;

    QWidget* centralWidget = window.centralWidget();

    QLabel* cameraPreview = window.cameraPreview();
    QLabel* statusLabel = window.statusLabel();
    QLabel* identity = window.identityLabel();
    QPushButton* startButton = window.startButton();
    QPushButton* stopButton = window.stopButton();
    CameraController camera;
    FaceDetector faceDetector;
    const QString modelPath = QCoreApplication::applicationDirPath()
        + "/models/haarcascade_frontalface_default.xml";
    if(!faceDetector.load(modelPath.toStdString()))
    {
        statusLabel->setText("状态：人脸检测模型加载失败");
    }

    QTimer*previewTimer=new QTimer(centralWidget);
    previewTimer->setInterval(33);
    QElapsedTimer detectionClock;
    std::vector<cv::Rect> detectedFaces;
    bool jpegSent=false;

    NetworkClient networkClient;
    QObject::connect(&networkClient,&NetworkClient::connected,statusLabel,
[statusLabel,&networkClient](){
        statusLabel->setText("状态：已连接服务器");
        networkClient.sendPayload("msg1");
        networkClient.sendPayload("msg2");
    });
    QObject::connect(&networkClient,&NetworkClient::connectionError,statusLabel,
        [statusLabel](const QString&errMsg){
        statusLabel->setText("状态：服务器连接失败  "+errMsg);
    });
    networkClient.connectToServer("127.0.0.1", 45454);

    QObject::connect(previewTimer,&QTimer::timeout,cameraPreview,
        [cameraPreview,&camera,statusLabel,&faceDetector,&detectionClock,
         &detectedFaces,identity,&jpegSent,&networkClient](){
        cv::Mat frame;
        if(!camera.read(frame)||frame.empty())
            {
            statusLabel->setText("读取摄像头画面失败");
            return;
        }
        frame=FrameProcessor::mirror(frame);
        if(!detectionClock.isValid() || detectionClock.elapsed() >= 100)
        {
            cv::Mat grayFrame=FrameProcessor::toGray(frame);
            faceDetector.detect(grayFrame,detectedFaces);
            detectionClock.restart();
            identity->setText(QString("检测到人脸数量：%1").arg(detectedFaces.size()));
        }
        QImage image =
            FrameProcessor::toPreviewImage(
                frame,
                detectedFaces);
        if(!jpegSent&&networkClient.isConnected())
        {
            QByteArray jpegBytes =
                FrameProcessor::encodeJpeg(image, 80);
            if (!jpegBytes.isEmpty())
            {
                QByteArray jpegPayload = "JPEG\n";
                jpegPayload.append(jpegBytes);
                if (networkClient.sendPayload(jpegPayload) >= 0)
                {
                    jpegSent = true;
                }
            }
        }

        cameraPreview->setPixmap(
            QPixmap::fromImage(image.copy()).scaled(
                cameraPreview->size(),Qt::KeepAspectRatio,Qt::FastTransformation));
    });

    QObject::connect(&window,&MainWindow::startCameraRequested,statusLabel,
            [statusLabel,startButton,stopButton,previewTimer,&camera,&faceDetector](){
        if(faceDetector.empty())
        {
            statusLabel->setText("状态：人脸检测模型不可用");
            return;
        }
        if(!camera.open(0)){
            statusLabel->setText("状态：摄像头打开失败");
            return;
        }
        statusLabel->setText("状态：已开启摄像头");
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        previewTimer->start();
    });

    QObject::connect(&window,&MainWindow::stopCameraRequested,statusLabel,
            [statusLabel,startButton,stopButton,previewTimer,cameraPreview,&camera](){
        statusLabel->setText("状态：摄像头已关闭");
        stopButton->setEnabled(false);
        startButton->setEnabled(true);
        previewTimer->stop();
        camera.release();
        cameraPreview->setText("相机预览");
    });

    window.show();

    return app.exec();
}
