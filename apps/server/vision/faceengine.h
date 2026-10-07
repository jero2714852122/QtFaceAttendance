#pragma once

#include <QByteArray>
#include <QString>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include <vector>

class FaceEngine
{
public:
    struct Face
    {
        cv::Rect box;
        cv::Mat embedding;
    };

    bool load(
        const QString& detectorModelPath,
        const QString& recognizerModelPath);

    bool analyze(
        const cv::Mat& image,
        std::vector<Face>& faces);

    bool analyzeJpeg(
        const QByteArray& jpegBytes,
        std::vector<Face>& faces);

    double similarity(
        const cv::Mat& firstEmbedding,
        const cv::Mat& secondEmbedding) const;

    // 特征向量与字节数组互转，用于把模板存进数据库
    static QByteArray toBytes(
        const cv::Mat& embedding);

    static cv::Mat fromBytes(
        const QByteArray& bytes);

    bool isLoaded() const;

    QString lastError() const;

private:
    cv::Ptr<cv::FaceDetectorYN> detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    QString lastError_;
};