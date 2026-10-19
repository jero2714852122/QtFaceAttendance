#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QHash>

#include "database/employeerepository.h"
#include "vision/faceengine.h"
#include "database/facetemplaterepository.h"
#include "database/attendancerepository.h"
class DatabaseManager;
class Server;
class ServerWindow;
class RecognitionWorker;
class QThread;


class ServerController : public QObject
{
    Q_OBJECT

public:
    ServerController(
        Server& server,
        ServerWindow& window,
        DatabaseManager& database,
        QObject* parent = nullptr);
    ~ServerController() override;


    bool initializeDatabase();
    void loadEmployees();
    void loadTemplates();
    bool initializeVision();
private slots:
    void onClientConnected(
        const QString& peer);

    void onClientDisconnected(
        const QString& peer);

    void onClientError(
        const QString& message);

    void onMessageReceived(
        const QString& peer,
        const QByteArray& message);

    void onAddEmployee(
        const QString& employeeNo,
        const QString& name,
        const QString& department);

    void onRefreshEmployees();

    void onDeleteEmployee(
        qint64 id);

    void onUpdateEmployee(
        qint64 id,
        const QString& name,
        const QString& department);

    void onRegisterFace(
        qint64 id);

    void onFrameAnalyzed(
        const QString& peer,
        const cv::Mat& embedding,
        qint64 elapsedMs);

    void onFrameAnalysisFailed(
        const QString& message);


private:
    // 一帧的分析结果。text 给人看，其余字段用来判断"状态变了没有"。
    // 判断状态不能拿 text 比：相似度每帧都在小幅抖动，文字跟着变，
    // 每帧都会被当成新状态。
    struct FrameResult
    {
        bool hasFace = false;
        bool matched = false;
        qint64 employeeId = 0;
        double score = 0.0;
        QString name;   // 识别成功时的姓名，回传给客户端显示
        QString text;   // 服务端日志里那一行
    };

    FrameResult recognize(
        const cv::Mat& embedding);

    // 返回一句可以直接显示的考勤结论，同时由调用方写进日志。
    QString recordAttendance(
        qint64 employeeId,
        double confidence);

    void sendResult(
        const QString& peer,
        const FrameResult& result,
        const QString& attendanceText);

    Server& server_;
    ServerWindow& window_;
    DatabaseManager& database_;
    EmployeeRepository employeeRepository_;
    FaceEngine faceEngine_;
    // 每个客户端最近一帧的人脸特征。
    QHash<QString, cv::Mat> lastEmbeddingByPeer_;

    // 最近一次收到人脸的客户端。登记时用它，并且要告诉操作者用的是谁。
    QString lastActivePeer_;

    FaceTemplateRepository faceTemplateRepository_;
    QList<FaceTemplate> templates_;
    AttendanceRepository attendanceRepository_;
    QHash<QString, FrameResult> lastResultByPeer_;
    QThread* analysisThread_ = nullptr;
    RecognitionWorker* analysisWorker_ = nullptr;


};
