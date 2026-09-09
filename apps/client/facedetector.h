#pragma once

#include <opencv2/objdetect.hpp>

#include <string>
#include <vector>

class FaceDetector
{
public:
    bool load(const std::string& modelPath);

    bool detect(
        const cv::Mat& grayFrame,
        std::vector<cv::Rect>& faces);
    bool empty() const;

private:
    cv::CascadeClassifier classifier_;
};