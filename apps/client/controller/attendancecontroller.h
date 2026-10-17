#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QTimer>

#include <opencv2/core.hpp>

#include <vector>

class MainWindow;
class CameraController;
class FaceDetector;
class NetworkClient;

class AttendanceController : public QObject
{
    Q_OBJECT

public:
    AttendanceController(
        MainWindow& window,
        CameraController& camera,
        FaceDetector& faceDetector,
        NetworkClient& networkClient,
        int cameraIndex,
        QObject* parent = nullptr);

public slots:
    void startCamera();
    void stopCamera();

private slots:
    void processFrame();
    void onNetworkConnected();
    void onNetworkError(const QString& errorMessage);
    void onServerMessage(const QByteArray& message);

private:
    MainWindow& window_;
    CameraController& camera_;
    FaceDetector& faceDetector_;
    NetworkClient& networkClient_;

    // 用哪个摄像头，来自 config.ini 的 client.camera_index。
    int cameraIndex_ = 0;

    QTimer previewTimer_;
    QElapsedTimer detectionClock_;
    std::vector<cv::Rect> detectedFaces_;

    // 上传限流用：靠它算"距上次上传过了多久"。
    QElapsedTimer frameSendClock_;
};
