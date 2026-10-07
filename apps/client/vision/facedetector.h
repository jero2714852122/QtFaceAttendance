#pragma once
#include <opencv2/objdetect.hpp>

#include <string>
#include <vector>

class FaceDetector
{
public:
    bool load(const std::string& modelPath);

    // 参数是彩色图。YuNet 是神经网络，喂灰度图会出错或给出垃圾结果。
    bool detect(
        const cv::Mat& bgrFrame,
        std::vector<cv::Rect>& faces);
    bool empty() const;

private:
    cv::Ptr<cv::FaceDetectorYN> detector_;
};
