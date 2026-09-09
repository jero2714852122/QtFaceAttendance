#include "cameracontroller.h"

CameraController::~CameraController()
{
    release();
}

bool CameraController::open(int deviceIndex)
{
    if (capture_.isOpened())
    {
        return true;
    }
    return capture_.open(deviceIndex);
}

bool CameraController::read(cv::Mat& frame)
{
    return capture_.read(frame);
}

void CameraController::release()
{
    if (capture_.isOpened())
    {
        capture_.release();
    }
}

bool CameraController::isOpened() const
{
    return capture_.isOpened();
}