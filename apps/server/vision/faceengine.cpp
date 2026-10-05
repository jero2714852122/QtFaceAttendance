#include "faceengine.h"

bool FaceEngine::load(
    const QString& detectorModelPath,
    const QString& recognizerModelPath)
{
    lastError_.clear();

    try
    {
        detector_ = cv::FaceDetectorYN::create(
            detectorModelPath.toStdString(),
            "",
            cv::Size(320, 320));
    }
    catch (const cv::Exception& error)
    {
        lastError_ =
            "YuNet 检测模型加载失败："
            + QString::fromStdString(error.msg);

        return false;
    }

    if (detector_.empty())
    {
        lastError_ =
            "YuNet 检测模型加载失败：返回了空指针";

        return false;
    }

    try
    {
        recognizer_ = cv::FaceRecognizerSF::create(
            recognizerModelPath.toStdString(),
            "");
    }
    catch (const cv::Exception& error)
    {
        lastError_ =
            "SFace 识别模型加载失败："
            + QString::fromStdString(error.msg);

        return false;
    }

    if (recognizer_.empty())
    {
        lastError_ =
            "SFace 识别模型加载失败：返回了空指针";

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