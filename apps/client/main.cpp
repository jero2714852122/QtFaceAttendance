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
    cv::CascadeClassifier faceDetector;
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

    QObject::connect(previewTimer,&QTimer::timeout,cameraPreview,
        [cameraPreview,&camera,statusLabel,&faceDetector,&detectionClock,
         &detectedFaces,identity](){
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
            faceDetector.detectMultiScale(
                grayFrame,
                detectedFaces,
                1.1,
                5,
                0,
                cv::Size(80,80)
                );
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

        cameraPreview->setPixmap(
            QPixmap::fromImage(image.copy()).scaled(
                cameraPreview->size(),Qt::KeepAspectRatio,Qt::FastTransformation));
    });

    QObject::connect(startButton,&QPushButton::clicked,statusLabel,
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
