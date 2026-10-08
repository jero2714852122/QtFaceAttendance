#include <QtTest>

#include "frameprotocol.h"

class FrameProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void packsTheLengthPrefix();
    void takesOneWholeFrame();
    void waitsForTheRestOfAFrame();
    void takesTwoFramesFromOneBuffer();
    void rejectsAnImpossibleLength();
};

void FrameProtocolTest::packsTheLengthPrefix()
{
    const QByteArray packet =
        FrameProtocol::pack("JPEG");

    // 长度写在最前面，4 个字节，大端序。大端的意思是高位在前，
    // 4 就该是 00 00 00 04，而不是 04 00 00 00。
    QCOMPARE(packet.size(), 8);
    QCOMPARE(packet.left(4), QByteArray("\x00\x00\x00\x04", 4));
    QCOMPARE(packet.mid(4), QByteArray("JPEG"));
}

void FrameProtocolTest::takesOneWholeFrame()
{
    QByteArray buffer = FrameProtocol::pack("hello");
    QByteArray frame;

    QCOMPARE(
        FrameProtocol::takeFrame(buffer, frame),
        FrameProtocol::FrameResult::Complete);

    QCOMPARE(frame, QByteArray("hello"));
    QVERIFY(buffer.isEmpty());
}

void FrameProtocolTest::waitsForTheRestOfAFrame()
{
    // 只到了一半：长度字段说后面有 5 个字节，实际只给了 2 个。
    // TCP 是字节流，这种情况每一帧都可能遇到，不是异常。
    QByteArray buffer = FrameProtocol::pack("hello").left(6);
    QByteArray frame;

    QCOMPARE(
        FrameProtocol::takeFrame(buffer, frame),
        FrameProtocol::FrameResult::Incomplete);

    QVERIFY(frame.isEmpty());

    // 半条不能被吃掉，否则补齐之后再也拼不回来。
    QCOMPARE(buffer.size(), 6);
}

void FrameProtocolTest::takesTwoFramesFromOneBuffer()
{
    // 两条消息粘在一起到达，这也是 TCP 的常态。
    QByteArray buffer =
        FrameProtocol::pack("one")
        + FrameProtocol::pack("two");

    QByteArray frame;

    QCOMPARE(
        FrameProtocol::takeFrame(buffer, frame),
        FrameProtocol::FrameResult::Complete);
    QCOMPARE(frame, QByteArray("one"));

    QCOMPARE(
        FrameProtocol::takeFrame(buffer, frame),
        FrameProtocol::FrameResult::Complete);
    QCOMPARE(frame, QByteArray("two"));

    QVERIFY(buffer.isEmpty());
}

void FrameProtocolTest::rejectsAnImpossibleLength()
{
    // 长度字段是 40 亿字节，远超上限。真实数据不可能这样，
    // 只可能是流已经错位，调用方收到 Invalid 就该断开连接。
    QByteArray buffer("\xFF\xFF\xFF\xFF", 4);
    QByteArray frame;

    QCOMPARE(
        FrameProtocol::takeFrame(buffer, frame),
        FrameProtocol::FrameResult::Invalid);
}

QTEST_APPLESS_MAIN(FrameProtocolTest)

#include "tst_frameprotocol.moc"
