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
#include<opencv2/imgproc.hpp>
#include<vector>
#include<opencv2/objdetect.hpp>
#include<QTcpSocket>
#include<QIODevice>
#include<QDataStream>
#include<QBuffer>
#include"mainwindow.h"
#include "cameracontroller.h"
#include"facedetector.h"
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


    QTcpSocket*attendencSocket=new QTcpSocket(centralWidget);
    QObject::connect(attendencSocket,&QTcpSocket::connected,statusLabel,
        [statusLabel,attendencSocket](){
            statusLabel->setText("状态：已连接服务器");
        QByteArray payLoad;
            auto appendpacket=[](const QByteArray&p){
                QByteArray packet;
                QDataStream output(&packet,QIODevice::WriteOnly);
                output.setByteOrder(QDataStream::BigEndian);
                output<<quint32(p.size());
                packet.append(p);
                return packet;
        };
        payLoad.append(appendpacket("msg1"));
        payLoad.append(appendpacket("msg2"));
        attendencSocket->write(payLoad);
    });


    QObject::connect(attendencSocket,&QTcpSocket::errorOccurred,statusLabel,
        [statusLabel,attendencSocket](){
        statusLabel->setText("状态：服务器连接失败  "+attendencSocket->errorString());
    });
    attendencSocket->connectToHost("127.0.0.1",45454);

    QObject::connect(previewTimer,&QTimer::timeout,cameraPreview,
        [cameraPreview,&camera,statusLabel,&faceDetector,&detectionClock,
         &detectedFaces,identity,&jpegSent,attendencSocket](){
        cv::Mat frame;
        if(!camera.read(frame)||frame.empty())
            {
            statusLabel->setText("读取摄像头画面失败");
            return;
        }
        cv::flip(frame,frame,1);
        if(!detectionClock.isValid() || detectionClock.elapsed() >= 100)
        {
            cv::Mat grayFrame;
            cv::cvtColor(frame,grayFrame,cv::COLOR_BGR2GRAY);
            cv::equalizeHist(grayFrame,grayFrame);
            faceDetector.detect(grayFrame,detectedFaces);
            detectionClock.restart();
            identity->setText(QString("检测到人脸数量：%1").arg(detectedFaces.size()));
        }
        for(const cv::Rect&face:detectedFaces)
        {
            cv::rectangle(frame,face,cv::Scalar(0,255,0),2);
        }

        cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);
        QImage image(
            frame.data,
            frame.cols,
            frame.rows,
            static_cast<qsizetype>(frame.step),
            QImage::Format_RGB888
            );
        if(!jpegSent&&attendencSocket->state()==QAbstractSocket::ConnectedState)
        {
            QByteArray jpegBytes;
            QBuffer buffer(&jpegBytes);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer,"JPG",80);

            QByteArray jpegPayload="JPEG\n";
            jpegPayload.append(jpegBytes);

            QByteArray jpegPacket;
            QDataStream output(&jpegPacket,QIODevice::WriteOnly);
            output.setByteOrder(QDataStream::BigEndian);
            output<<quint32(jpegPayload.size());
            jpegPacket.append(jpegPayload);
            attendencSocket->write(jpegPacket);
            jpegSent=true;
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
