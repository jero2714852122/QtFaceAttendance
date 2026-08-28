#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include<QWidget>
#include<QVBoxLayout>
#include<QPushButton>
#include<QHBoxLayout>
#include<QObject>
#include<QTimer>
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
    QTimer*previewTimer=new QTimer(centralWidget);
    previewTimer->setInterval(500);
    QObject::connect(previewTimer,&QTimer::timeout,cameraPreview,
        [cameraPreview,markerVisible=false]()mutable{
        markerVisible=!markerVisible;
        cameraPreview->setText(markerVisible?" 相机预览（更新中）" : "相机预览");
    });


    QObject::connect(startButton,&QPushButton::clicked,statusLabel,[statusLabel,startButton,stopButton,previewTimer](){
        statusLabel->setText("状态：已请求摄像头");
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        previewTimer->start();
    });
    QObject::connect(stopButton,&QPushButton::clicked,statusLabel,[statusLabel,startButton,stopButton,previewTimer,cameraPreview](){
        statusLabel->setText("状态：摄像头已关闭");
        stopButton->setEnabled(false);
        startButton->setEnabled(true);
        previewTimer->stop();
        cameraPreview->setText("相机预览");
    });
    window.setCentralWidget(centralWidget);
    window.show();

    return app.exec();
}
