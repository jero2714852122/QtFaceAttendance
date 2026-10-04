#pragma once

#include <QString>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

class FaceEngine
{
public:
    bool load(
        const QString& detectorModelPath,
        const QString& recognizerModelPath);

    bool isLoaded() const;

    QString lastError() const;

private:
    cv::Ptr<cv::FaceDetectorYN> detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    QString lastError_;
};