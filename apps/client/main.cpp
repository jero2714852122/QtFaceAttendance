#include <QApplication>
#include<QObject>
#include<QTimer>
#include<opencv2/videoio.hpp>
#include<QImage>
#include <QCoreApplication>
#include <QElapsedTimer>
#include<vector>
#include<opencv2/objdetect.hpp>
#include"mainwindow.h"
#include "cameracontroller.h"
#include"facedetector.h"
#include"networkclient.h"
#include "frameprocessor.h"
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;


    CameraController camera;
    FaceDetector faceDetector;
    const QString modelPath = QCoreApplication::applicationDirPath()
        + "/models/haarcascade_frontalface_default.xml";
    if(!faceDetector.load(modelPath.toStdString()))
    {
        window.setStatusText("状态：人脸检测模型加载失败");
    }

    QTimer*previewTimer=new QTimer(&window);
    previewTimer->setInterval(33);
    QElapsedTimer detectionClock;
    std::vector<cv::Rect> detectedFaces;
    bool jpegSent=false;

    NetworkClient networkClient;
    QObject::connect(&networkClient,&NetworkClient::connected,&window,
[&window,&networkClient](){
        window.setStatusText("状态：已连接服务器");
        networkClient.sendPayload("msg1");
        networkClient.sendPayload("msg2");
    });
    QObject::connect(&networkClient,&NetworkClient::connectionError,&window,
        [&window](const QString&errMsg){
        window.setStatusText("状态：服务器连接失败  "+errMsg);
    });
    networkClient.connectToServer("127.0.0.1", 45454);

    QObject::connect(previewTimer,&QTimer::timeout,&window,
        [&window,&camera,&faceDetector,&detectionClock,
         &detectedFaces,&jpegSent,&networkClient](){
        cv::Mat frame;
        if(!camera.read(frame)||frame.empty())
            {
            window.setStatusText("读取摄像头画面失败");
            return;
        }
        frame=FrameProcessor::mirror(frame);
        if(!detectionClock.isValid() || detectionClock.elapsed() >= 100)
        {
            cv::Mat grayFrame=FrameProcessor::toGray(frame);
            faceDetector.detect(grayFrame,detectedFaces);
            detectionClock.restart();
            window.setFaceCount(
                static_cast<int>(detectedFaces.size()));
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

        window.showPreviewImage(image);
    });

    QObject::connect(&window,&MainWindow::startCameraRequested,&window,
            [&window,previewTimer,&camera,&faceDetector](){
        if(faceDetector.empty())
        {
            window.setStatusText("状态：人脸检测模型不可用");
            return;
        }
        if(!camera.open(0)){
            window.setStatusText("状态：摄像头打开失败");
            return;
        }
        window.setStatusText("状态：已开启摄像头");
        window.setCameraRunning(true);
        previewTimer->start();
    });

    QObject::connect(&window,&MainWindow::stopCameraRequested,&window,
            [&window,previewTimer,&camera](){
        window.setStatusText("状态：摄像头已关闭");
        window.setCameraRunning(false);

        previewTimer->stop();
        camera.release();
        window.resetPreview();
    });

    window.show();

    return app.exec();
}
