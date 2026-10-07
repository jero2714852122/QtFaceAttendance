#include "facedetector.h"

#include <opencv2/core.hpp>

namespace
{
// 和服务端保持一致。两边阈值不同，就会出现「客户端框住了、服务端却说
// 没检测到」这种自相矛盾的画面。
constexpr float kDetectionScoreThreshold = 0.7f;

// create() 要求一个初始输入尺寸，实际每帧会被 setInputSize 覆盖。
constexpr int kInitialInputSize = 320;
}

bool FaceDetector::load(const std::string& modelPath)
{
    try
    {
        detector_ = cv::FaceDetectorYN::create(
            modelPath,
            "",
            cv::Size(kInitialInputSize, kInitialInputSize),
            kDetectionScoreThreshold);
    }
    catch (const cv::Exception&)
    {
        // 模型缺失或损坏时 create() 抛异常。客户端是 GUI 程序，
        // 异常冲出 main() 就是 abort，连窗口都出不来，必须在这里接住。
        return false;
    }

    return !detector_.empty();
}

bool FaceDetector::detect(
    const cv::Mat& bgrFrame,
    std::vector<cv::Rect>& faces)
{
    faces.clear();

    if (detector_.empty() || bgrFrame.empty())
    {
        return false;
    }

    // 神经网络检测器每帧都要重新告知输入尺寸，画面分辨率变了不会自动跟上。
    detector_->setInputSize(bgrFrame.size());

    cv::Mat detections;

    detector_->detect(bgrFrame, detections);

    // 输出是 N 行 15 列：前 4 列是框，后面是关键点和置信度。
    // 这里只要框，用来在预览上画绿框。
    for (int row = 0; row < detections.rows; ++row)
    {
        faces.push_back(cv::Rect(
            static_cast<int>(detections.at<float>(row, 0)),
            static_cast<int>(detections.at<float>(row, 1)),
            static_cast<int>(detections.at<float>(row, 2)),
            static_cast<int>(detections.at<float>(row, 3))));
    }

    return true;
}

bool FaceDetector::empty() const
{
    return detector_.empty();
}
