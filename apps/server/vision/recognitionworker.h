#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <opencv2/core.hpp>

#include "faceengine.h"

// 这里不需要 Q_DECLARE_METATYPE(cv::Mat)。
//
// Qt 5 时代，自定义类型要进信号槽必须先写那行宏，否则跨线程的队列连接
// 会把参数丢掉、并在运行时打个警告。
// Qt 6 改了规则：只要类型有默认构造、可拷贝、可析构，元类型系统就能自动
// 处理，moc 生成的代码会自己注册。
//
// 在 Qt 6 里再写那行宏反而编不过，MSVC 报 C2908「显式专用化；已实例化」
// —— 因为自动机制已经先实例化过了，你的显式声明成了重复定义。

class RecognitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit RecognitionWorker(QObject* parent = nullptr);

     bool load(
        const QString& detectorModelPath,
        const QString& recognizerModelPath);

    QString lastError() const;

public slots:
    // 这个方法在 worker 线程里执行。
    void analyze(
        const QString& peer,
        const QByteArray& jpegFrame);

signals:
    // 这两个信号发出去之后会在主线程执行，取决于连接方式。
    // embedding 为空表示这一帧没检测到人脸。
    void analyzed(
        const QString& peer,
        const cv::Mat& embedding,
        qint64 elapsedMs);

    void failed(
        const QString& message);

private:
    FaceEngine engine_;
};
