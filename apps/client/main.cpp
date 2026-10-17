#include <QApplication>
#include <QCoreApplication>

#include "appconfig.h"
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

    // 配置先读一次，后面要用地址端口的地方都从它取。
    const AppConfig config = AppConfig::load();

    // 客户端只有一个状态栏标签，配置问题合成一行显示。
    // 注意：下面模型加载失败时会盖掉它，因为那是更致命的问题。
    if (!config.notes().isEmpty())
    {
        window.setStatusText(
            QStringLiteral("配置：")
            + config.notes().join(QStringLiteral("；")));
    }

    //资源加载，提供依赖
    const QString modelPath =
        QCoreApplication::applicationDirPath()
        + "/models/face_detection_yunet_2023mar.onnx";

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
        config.client().cameraIndex,
        &window);
    //启动网络连接：地址和端口来自 config.ini
    networkClient.connectToServer(
        config.client().serverAddress,
        config.client().serverPort);

    window.show();
    return app.exec();
}
