#include "faceengine.h"

bool FaceEngine::load(
    const QString& detectorModelPath,
    const QString& recognizerModelPath)
{
    lastError_.clear();

    detector_ = cv::FaceDetectorYN::create(
        detectorModelPath.toStdString(),
        "",
        cv::Size(320, 320));

    if (detector_.empty())
    {
        lastError_ =
            "YuNet 检测模型加载失败：" + detectorModelPath;

        return false;
    }

    recognizer_ = cv::FaceRecognizerSF::create(
        recognizerModelPath.toStdString(),
        "");

    if (recognizer_.empty())
    {
        lastError_ =
            "SFace 识别模型加载失败：" + recognizerModelPath;

        return false;
    }

    return true;
}

bool FaceEngine::isLoaded() const
{
    return !detector_.empty()
    && !recognizer_.empty();
}

QString FaceEngine::lastError() const
{
    return lastError_;
}