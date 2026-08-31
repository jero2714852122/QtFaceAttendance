#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include<QTcpServer>
#include<QHostAddress>
#include<QTcpSocket>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Face Attendance Server");
    window.setMinimumSize(960, 640);

    auto* status = new QLabel("Server environment ready", &window);
    status->setAlignment(Qt::AlignCenter);
    QTcpServer server;
    if(server.listen(QHostAddress::LocalHost,45454))
    {
        status->setText("服务器正在监听127.0.0.1:45454");
    }
    else
        status->setText("服务器监听失败"+server.errorString());
    QObject::connect(&server,&QTcpServer::newConnection,&server,
        [&server,status](){
        QTcpSocket*clientSocket=server.nextPendingConnection();
        status->setText("客户端已连接："+clientSocket->peerAddress().toString());
        QObject::connect(clientSocket,&QTcpSocket::readyRead,clientSocket,[clientSocket,status]
            (){QByteArray msg=clientSocket->readAll();
            status->setText("收到消息："+QString::fromUtf8(msg));}   );
        QObject::connect(
        clientSocket,
        &QTcpSocket::disconnected,
        clientSocket,
        &QTcpSocket::deleteLater);}
    );

    window.setCentralWidget(status);

    window.show();

    return app.exec();
}
