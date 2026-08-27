#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include<QWidget>
#include<QVBoxLayout>
#include<QPushButton>
#include<QHBoxLayout>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("face attendance system");
    window.setMinimumSize(960, 640);

    QWidget*centralWidget=new QWidget(&window);
    QVBoxLayout*mainLayout=new QVBoxLayout(centralWidget);
    QLabel*cameraPreview=new QLabel("cameraPreview",centralWidget);
    cameraPreview->setAlignment(Qt::AlignCenter);
    cameraPreview->setMinimumHeight(360);
    mainLayout->addWidget(cameraPreview,1);
    QLabel*statusLabel=new QLabel("Status: Waiting for camera",centralWidget);
    statusLabel->setMinimumHeight(40);
    statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    mainLayout->addWidget(statusLabel);
    QLabel*identity=new QLabel("Identity: Not recognized",centralWidget);
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

    window.setCentralWidget(centralWidget);
    window.show();

    return app.exec();
}
