#pragma once

#include <QByteArray>
#include <QImage>
#include <opencv2/core.hpp>
#include <vector>

class FrameProcessor
{
public:

    static cv::Mat mirror(const cv::Mat& bgrFrame);

    static cv::Mat toGray(const cv::Mat& bgrFrame);

    static QImage toPreviewImage(
        const cv::Mat& bgrFrame,
        const std::vector<cv::Rect>& faces);

    static QByteArray encodeJpeg(
        const QImage& image,
        int quality = 80);
};
