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

    jpegSent_ = false;
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
        cv::Mat grayFrame =
            FrameProcessor::toGray(frame);

        faceDetector_.detect(
            grayFrame,
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

    if (!jpegSent_ &&
        networkClient_.isConnected())
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
                jpegSent_ = true;
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