#include "frameprocessor.h"

#include <QBuffer>
#include <QIODevice>
#include <opencv2/imgproc.hpp>

cv::Mat FrameProcessor::mirror(const cv::Mat& bgrFrame)
{
    cv::Mat mirroredFrame;

    cv::flip(bgrFrame, mirroredFrame, 1);

    return mirroredFrame;
}

cv::Mat FrameProcessor::toGray(const cv::Mat& bgrFrame)
{
    cv::Mat grayFrame;

    cv::cvtColor(
        bgrFrame,
        grayFrame,
        cv::COLOR_BGR2GRAY);

    cv::equalizeHist(
        grayFrame,
        grayFrame);
    return grayFrame;
}

QImage FrameProcessor::toPreviewImage(
    const cv::Mat& bgrFrame,
    const std::vector<cv::Rect>& faces)
{
    cv::Mat displayFrame = bgrFrame.clone();
    for (const cv::Rect& face : faces)
    {
        cv::rectangle(
            displayFrame,
            face,
            cv::Scalar(0, 255, 0),
            2);
    }
    cv::Mat rgbFrame;
    cv::cvtColor(
        displayFrame,
        rgbFrame,
        cv::COLOR_BGR2RGB);
    QImage image(
        rgbFrame.data,
        rgbFrame.cols,
        rgbFrame.rows,
        static_cast<qsizetype>(rgbFrame.step),
        QImage::Format_RGB888);
    return image.copy();
}

QByteArray FrameProcessor::encodeJpeg(
    const QImage& image,
    int quality)
{
    QByteArray jpegBytes;
    QBuffer buffer(&jpegBytes);
    buffer.open(QIODevice::WriteOnly);

    if (!image.save(&buffer, "JPG", quality))
    {
        return {};
    }

    return jpegBytes;
}