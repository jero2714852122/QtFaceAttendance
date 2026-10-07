#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include "database/employeerepository.h"
#include "vision/faceengine.h"
#include "database/facetemplaterepository.h"
class DatabaseManager;
class Server;
class ServerWindow;

class ServerController : public QObject
{
    Q_OBJECT

public:
    ServerController(
        Server& server,
        ServerWindow& window,
        DatabaseManager& database,
        QObject* parent = nullptr);

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

private:
    QString describeRecognition(
        const cv::Mat& embedding);
    Server& server_;
    ServerWindow& window_;
    DatabaseManager& database_;
    EmployeeRepository employeeRepository_;
    FaceEngine faceEngine_;
    cv::Mat lastEmbedding_;
    FaceTemplateRepository faceTemplateRepository_;
    QList<FaceTemplate> templates_;
    QString lastRecognitionResult_;

};
