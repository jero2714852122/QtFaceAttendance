#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include<opencv2/videoio.hpp>
class CameraController:public QObject
{
    Q_OBJECT
public:
    ~CameraController();
    bool open(int deviceIndex=0);
    bool read(cv::Mat& frame);
    void release();
    bool isOpened() const;
private:
    cv::VideoCapture capture_;
};

#endif
