#include "faceengine.h"
#include <opencv2/imgcodecs.hpp>
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
bool FaceEngine::analyze(
    const cv::Mat& image,
    std::vector<Face>& faces)
{
    faces.clear();

    if (!isLoaded())
    {
        lastError_ = "人脸模型尚未加载";

        return false;
    }

    if (image.empty())
    {
        lastError_ = "待分析的图像为空";

        return false;
    }

    try
    {
        detector_->setInputSize(image.size());

        cv::Mat detections;

        detector_->detect(image, detections);

        if (detections.empty())
        {
            return true;
        }

        for (int row = 0; row < detections.rows; ++row)
        {
            Face face;

            face.box = cv::Rect(
                static_cast<int>(
                    detections.at<float>(row, 0)),
                static_cast<int>(
                    detections.at<float>(row, 1)),
                static_cast<int>(
                    detections.at<float>(row, 2)),
                static_cast<int>(
                    detections.at<float>(row, 3)));

            cv::Mat alignedFace;

            recognizer_->alignCrop(
                image,
                detections.row(row),
                alignedFace);

            recognizer_->feature(
                alignedFace,
                face.embedding);

            faces.push_back(face);
        }
    }
    catch (const cv::Exception& error)
    {
        lastError_ =
            "人脸分析失败："
            + QString::fromStdString(error.msg);

        faces.clear();

        return false;
    }

    return true;
}

bool FaceEngine::analyzeJpeg(
    const QByteArray& jpegBytes,
    std::vector<Face>& faces)
{
    faces.clear();

    if (jpegBytes.isEmpty())
    {
        lastError_ = "图像数据为空";
        return false;
    }

    std::vector<uchar> buffer(
        jpegBytes.begin(),
        jpegBytes.end());

    const cv::Mat image = cv::imdecode(
        buffer,
        cv::IMREAD_COLOR);

    if (image.empty())
    {
        lastError_ = "JPEG 解码失败";

        return false;
    }

    return analyze(image, faces);
}

double FaceEngine::similarity(
    const cv::Mat& firstEmbedding,
    const cv::Mat& secondEmbedding) const
{
    if (firstEmbedding.empty()
        || secondEmbedding.empty())
    {
        return 0.0;
    }

    return recognizer_->match(
        firstEmbedding,
        secondEmbedding,
        cv::FaceRecognizerSF::DisType::FR_COSINE);
}
QString FaceEngine::lastError() const
{
    return lastError_;
}