#include <QApplication>
#include <QCoreApplication>

#include "camera/cameracontroller.h"
#include "controller/attendancecontroller.h"
#include "network/networkclient.h"
#include "ui/mainwindow.h"
#include "vision/facedetector.h"

int main(int argc, char* argv[])
{
    //创建各个模块的对象，程序组装
    QApplication app(argc, argv);
    MainWindow window;
    CameraController camera;
    FaceDetector faceDetector;
    NetworkClient networkClient;
    //资源加载，提供依赖
    const QString modelPath =
        QCoreApplication::applicationDirPath()
        + "/models/haarcascade_frontalface_default.xml";

    if (!faceDetector.load(
            modelPath.toStdString()))
    {
        window.setStatusText(
            "状态：人脸检测模型加载失败");
    }
    //组装控制器
    AttendanceController controller(
        window,
        camera,
        faceDetector,
        networkClient,
        &window);
    //启动网络连接
    networkClient.connectToServer(
        "127.0.0.1",
        45454);

    window.show();
    return app.exec();
}