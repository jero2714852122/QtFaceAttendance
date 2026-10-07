#include "faceengine.h"
#include <opencv2/imgcodecs.hpp>
#include <cstring>

namespace
{
// SFace 的输出固定是 1x128 的 float 特征向量，也就是 512 字节。
constexpr int kEmbeddingColumns = 128;
}

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

QByteArray FaceEngine::toBytes(
    const cv::Mat& embedding)
{
    if (embedding.empty()
        || embedding.type() != CV_32F)
    {
        return {};
    }

    // Mat 的特征数据是连续存放的 float，直接按字节整块取出来即可，
    // 不需要逐个元素遍历。
    return QByteArray(
        reinterpret_cast<const char*>(
            embedding.ptr<float>()),
        static_cast<int>(
            embedding.total() * sizeof(float)));
}

cv::Mat FaceEngine::fromBytes(
    const QByteArray& bytes)
{
    const int expectedSize =
        kEmbeddingColumns
        * static_cast<int>(sizeof(float));

    // 数据库里的内容可能被外部工具改坏，长度不对就不能硬拷贝，
    // 否则 memcpy 会越过 Mat 的缓冲区边界。
    if (bytes.size() != expectedSize)
    {
        return {};
    }

    cv::Mat embedding(
        1,
        kEmbeddingColumns,
        CV_32F);

    std::memcpy(
        embedding.ptr<float>(),
        bytes.constData(),
        static_cast<size_t>(expectedSize));

    return embedding;
}


QString FaceEngine::lastError() const
{
    return lastError_;
}