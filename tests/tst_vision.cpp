#include <QtTest>

#include <QByteArray>

#include "vision/faceengine.h"

class VisionTest : public QObject
{
    Q_OBJECT

private slots:
    void rejectsMissingModelFiles();
    void roundTripsEmbeddingBytes();
    void rejectsCorruptEmbeddingBytes();
};

void VisionTest::rejectsMissingModelFiles()
{
    FaceEngine engine;

    // 两个都不存在的路径。加载必须失败，而且要给出可读的错误信息——
    // 服务端启动时会把这句打到状态区，用户就是靠它发现"模型放错地方了"。
    QVERIFY(!engine.load(
        QStringLiteral("Z:/nowhere/no_such_detector.onnx"),
        QStringLiteral("Z:/nowhere/no_such_recognizer.onnx")));

    QVERIFY(!engine.lastError().isEmpty());
    QVERIFY(!engine.isLoaded());
}

void VisionTest::roundTripsEmbeddingBytes()
{
    // SFace 输出的特征就是 128 个 float。这里造一个假的，测序列化往返。
    constexpr int kColumns = 128;

    cv::Mat original(1, kColumns, CV_32F);

    for (int i = 0; i < kColumns; ++i)
    {
        original.at<float>(0, i) =
            static_cast<float>(i) * 0.25f;
    }

    const QByteArray bytes =
        FaceEngine::toBytes(original);

    QCOMPARE(
        bytes.size(),
        kColumns * static_cast<int>(sizeof(float)));

    const cv::Mat restored =
        FaceEngine::fromBytes(bytes);

    QCOMPARE(restored.rows, original.rows);
    QCOMPARE(restored.cols, original.cols);
    QCOMPARE(restored.type(), original.type());

    // 逐个元素比。特征要拿去做余弦相似度，差一个 bit 都会影响分数。
    for (int i = 0; i < kColumns; ++i)
    {
        QCOMPARE(
            restored.at<float>(0, i),
            original.at<float>(0, i));
    }
}

void VisionTest::rejectsCorruptEmbeddingBytes()
{
    constexpr int kColumns = 128;
    constexpr int kExpectedSize =
        kColumns * static_cast<int>(sizeof(float));

    // 长度不对的一律拒绝。库里的数据被外部工具改坏时，
    // 硬 memcpy 会越过缓冲区边界——那是最难查的崩溃。
    QVERIFY(FaceEngine::fromBytes(QByteArray()).empty());
    QVERIFY(FaceEngine::fromBytes(QByteArray(3, 'x')).empty());
    QVERIFY(FaceEngine::fromBytes(QByteArray(kExpectedSize - 1, 0)).empty());
    QVERIFY(FaceEngine::fromBytes(QByteArray(kExpectedSize + 1, 0)).empty());

    // 长度正好对的会被接受——但内容是不是真特征，它管不了。
    // 这是当前实现的边界：只校验长度，不校验内容。
    // 要更严得再加一层（比如向量的模长落在合理范围）。
    QVERIFY(!FaceEngine::fromBytes(QByteArray(kExpectedSize, 0)).empty());
}

QTEST_GUILESS_MAIN(VisionTest)

#include "tst_vision.moc"
