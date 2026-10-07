#include "attendancecontroller.h"

#include "camera/cameracontroller.h"
#include "network/networkclient.h"
#include "ui/mainwindow.h"
#include "vision/facedetector.h"
#include "vision/frameprocessor.h"

#include <QByteArray>
#include <QImage>

AttendanceController::AttendanceController(
    MainWindow& window,
    CameraController& camera,
    FaceDetector& faceDetector,
    NetworkClient& networkClient,
    QObject* parent)
    : QObject(parent)
    , window_(window)
    , camera_(camera)
    , faceDetector_(faceDetector)
    , networkClient_(networkClient)
    , previewTimer_(this)
{
    previewTimer_.setInterval(33);

    QObject::connect(
        &window_,
        &MainWindow::startCameraRequested,
        this,
        &AttendanceController::startCamera);

    QObject::connect(
        &window_,
        &MainWindow::stopCameraRequested,
        this,
        &AttendanceController::stopCamera);

    QObject::connect(
        &previewTimer_,
        &QTimer::timeout,
        this,
        &AttendanceController::processFrame);

    QObject::connect(
        &networkClient_,
        &NetworkClient::connected,
        this,
        &AttendanceController::onNetworkConnected);

    QObject::connect(
        &networkClient_,
        &NetworkClient::connectionError,
        this,
        &AttendanceController::onNetworkError);
}

void AttendanceController::startCamera()
{
    if (faceDetector_.empty())
    {
        window_.setStatusText(
            "状态：人脸检测模型不可用");
        return;
    }

    if (!camera_.open(0))
    {
        window_.setStatusText(
            "状态：摄像头打开失败");
        return;
    }

    frameSendClock_.invalidate();
    detectedFaces_.clear();
    detectionClock_.invalidate();

    window_.setStatusText(
        "状态：已开启摄像头");
    window_.setCameraRunning(true);

    previewTimer_.start();
}

void AttendanceController::stopCamera()
{
    previewTimer_.stop();
    camera_.release();

    window_.setStatusText(
        "状态：摄像头已关闭");
    window_.setCameraRunning(false);
    window_.resetPreview();
}

void AttendanceController::processFrame()
{
    cv::Mat frame;

    if (!camera_.read(frame) || frame.empty())
    {
        window_.setStatusText(
            "读取摄像头画面失败");
        return;
    }

    frame = FrameProcessor::mirror(frame);

    if (!detectionClock_.isValid() ||
        detectionClock_.elapsed() >= 100)
    {
        // YuNet 要的是彩色图，不能像 Haar 那样先转灰度。
        faceDetector_.detect(
            frame,
            detectedFaces_);

        detectionClock_.restart();

        window_.setFaceCount(
            static_cast<int>(
                detectedFaces_.size()));
    }

    QImage image =
        FrameProcessor::toPreviewImage(
            frame,
            detectedFaces_);

    // 预览定时器大约每 30 毫秒触发一次。逐帧上传会把带宽和 CPU 打满，
    // 而人脸在半秒内不会变成另一个人，所以按固定间隔抽样上传。
    constexpr qint64 kFrameSendIntervalMs = 500;

    const bool sendDue =
        !frameSendClock_.isValid()
        || frameSendClock_.elapsed() >= kFrameSendIntervalMs;

    // 这里刻意不再要求"客户端检测到脸才发"。客户端用的检测器比服务端的
    // 弱，一漏检画面就断流；而且服务端收不到"没人脸"的帧，就永远不知道
    // 人已经走开了。用带宽换正确性，本机演示完全负担得起。
    if (networkClient_.isConnected() && sendDue)
    {
        QByteArray jpegBytes =
            FrameProcessor::encodeJpeg(
                image,
                80);

        if (!jpegBytes.isEmpty())
        {
            QByteArray jpegPayload = "JPEG\n";
            jpegPayload.append(jpegBytes);

            if (networkClient_.sendPayload(
                    jpegPayload) >= 0)
            {
                // 只有发送成功才重置计时。失败就留着过期的计时器，
                // 让下一帧立刻重试，而不是白等半秒。
                frameSendClock_.restart();
            }
        }
    }

    window_.showPreviewImage(image);
}

void AttendanceController::onNetworkConnected()
{
    window_.setStatusText(
        "状态：已连接服务器");

    networkClient_.sendPayload("msg1");
    networkClient_.sendPayload("msg2");
}

void AttendanceController::onNetworkError(
    const QString& errorMessage)
{
    window_.setStatusText(
        "状态：服务器连接失败  "
        + errorMessage);
}
