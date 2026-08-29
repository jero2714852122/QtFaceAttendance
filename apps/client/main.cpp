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
#include<opencv2/imgproc.hpp>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("人脸考勤系统");
    window.setMinimumSize(960, 640);

    QWidget*centralWidget=new QWidget(&window);
    QVBoxLayout*mainLayout=new QVBoxLayout(centralWidget);
    QLabel*cameraPreview=new QLabel("相机预览",centralWidget);
    cameraPreview->setAlignment(Qt::AlignCenter);
    cameraPreview->setMinimumHeight(360);
    mainLayout->addWidget(cameraPreview,1);
    QLabel*statusLabel=new QLabel("状态：等待摄像头开启",centralWidget);
    statusLabel->setMinimumHeight(40);
    statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    mainLayout->addWidget(statusLabel);
    QLabel*identity=new QLabel("身份：未能识别身份",centralWidget);
    identity->setMinimumHeight(40);
    identity->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    mainLayout->addWidget(identity);
    QHBoxLayout*actionLayout=new QHBoxLayout;
    QPushButton*startButton=new QPushButton("打开摄像头",centralWidget);
    QPushButton*stopButton=new QPushButton("关闭摄像头",centralWidget);
    stopButton->setEnabled(false);
    actionLayout->addWidget(startButton);
    actionLayout->addWidget(stopButton);
    mainLayout->addLayout(actionLayout);

    cv::VideoCapture camera;
    QTimer*previewTimer=new QTimer(centralWidget);
    previewTimer->setInterval(33);

    QObject::connect(previewTimer,&QTimer::timeout,cameraPreview,
        [cameraPreview,&camera,statusLabel](){
        cv::Mat frame;
        if(!camera.read(frame)||frame.empty())
            {
            statusLabel->setText("读取摄像头画面失败");
            return;
        }
        cv::flip(frame,frame,1);
        cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);

        QImage image(
            frame.data,
            frame.cols,
            frame.rows,
            static_cast<qsizetype>(frame.step),
            QImage::Format_RGB888
            );

        cameraPreview->setPixmap(
            QPixmap::fromImage(image.copy()).scaled(
                cameraPreview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    });

    QObject::connect(startButton,&QPushButton::clicked,statusLabel,
            [statusLabel,startButton,stopButton,previewTimer,&camera](){
        if(!camera.open(0)){
            statusLabel->setText("状态：摄像头打开失败");
            return;
        }
        statusLabel->setText("状态：已开启摄像头");
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        previewTimer->start();
    });

    QObject::connect(stopButton,&QPushButton::clicked,statusLabel,
            [statusLabel,startButton,stopButton,previewTimer,cameraPreview,&camera](){
        statusLabel->setText("状态：摄像头已关闭");
        stopButton->setEnabled(false);
        startButton->setEnabled(true);
        previewTimer->stop();
        camera.release();
        cameraPreview->setText("相机预览");
    });
    window.setCentralWidget(centralWidget);
    window.show();

    return app.exec();
}
