#include "recognitionworker.h"

#include <QElapsedTimer>

RecognitionWorker::RecognitionWorker(
    QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

bool RecognitionWorker::load(
    const QString& detectorModelPath,
    const QString& recognizerModelPath)
{
    return engine_.load(
        detectorModelPath,
        recognizerModelPath);
}

QString RecognitionWorker::lastError() const
{
    return engine_.lastError();
}

void RecognitionWorker::analyze(
    const QString& peer,
    const QByteArray& jpegFrame)
{
    QElapsedTimer clock;
    clock.start();

    std::vector<FaceEngine::Face> faces;

    if (!engine_.analyzeJpeg(jpegFrame, faces))
    {
        emit failed(engine_.lastError());

        return;
    }

    cv::Mat embedding;

    if (!faces.empty())
    {
        // 只取可信度最高的那张。考勤场景要求一人入镜，多张脸属于异常输入。
        const FaceEngine::Face* bestFace = &faces.front();

        for (const FaceEngine::Face& face : faces)
        {
            if (face.score > bestFace->score)
            {
                bestFace = &face;
            }
        }

        // 必须 clone。特征存在 faces 这块内存里，函数一返回就可能被
        // 下次分析复用。只把头带出去，数据会跟着变。
        embedding = bestFace->embedding.clone();
    }

    emit analyzed(
        peer,
        embedding,
        clock.elapsed());
}
