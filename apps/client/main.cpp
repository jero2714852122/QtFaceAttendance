#include <QApplication>
#include <QCoreApplication>

#include "camera/cameracontroller.h"
#include "controller/attendancecontroller.h"
#include "network/networkclient.h"
#include "ui/mainwindow.h"
#include "vision/facedetector.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    CameraController camera;
    FaceDetector faceDetector;
    NetworkClient networkClient;

    const QString modelPath =
        QCoreApplication::applicationDirPath()
        + "/models/haarcascade_frontalface_default.xml";

    if (!faceDetector.load(
            modelPath.toStdString()))
    {
        window.setStatusText(
            "状态：人脸检测模型加载失败");
    }

    AttendanceController controller(
        window,
        camera,
        faceDetector,
        networkClient,
        &window);

    networkClient.connectToServer(
        "127.0.0.1",
        45454);

    window.show();

    return app.exec();
}