#include "facedetector.h"

bool FaceDetector::load(const std::string& modelPath)
{
    return classifier_.load(modelPath);
}

bool FaceDetector::detect(
    const cv::Mat& grayFrame,
    std::vector<cv::Rect>& faces)
{
    if (classifier_.empty())
    {
        faces.clear();
        return false;
    }

    classifier_.detectMultiScale(
        grayFrame,
        faces,
        1.1,
        5,
        0,
        cv::Size(80, 80));

    return true;
}

bool FaceDetector::empty() const
{
    return classifier_.empty();
}