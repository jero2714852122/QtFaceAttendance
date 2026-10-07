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
        QObject* parent = nullptr);

public slots:
    void startCamera();
    void stopCamera();

private slots:
    void processFrame();
    void onNetworkConnected();
    void onNetworkError(const QString& errorMessage);

private:
    MainWindow& window_;
    CameraController& camera_;
    FaceDetector& faceDetector_;
    NetworkClient& networkClient_;

    QTimer previewTimer_;
    QElapsedTimer detectionClock_;
    std::vector<cv::Rect> detectedFaces_;

    // 上传限流用：靠它算"距上次上传过了多久"。
    QElapsedTimer frameSendClock_;
};
