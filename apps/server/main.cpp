#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include<QTcpServer>
#include<QHostAddress>
#include<QTcpSocket>
#include<QDataStream>
#include<QImage>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Face Attendance Server");
    window.setMinimumSize(960, 640);

    auto* status = new QLabel("Server environment ready", &window);
    status->setAlignment(Qt::AlignCenter);
    QTcpServer server;
    QByteArray receiveBuffer;


    if(server.listen(QHostAddress::LocalHost,45454))
    {
        status->setText("服务器正在监听127.0.0.1:45454");
    }
    else
        status->setText("服务器监听失败"+server.errorString());
    QObject::connect(&server,&QTcpServer::newConnection,&server,
        [&server,status,&receiveBuffer](){
        QTcpSocket*clientSocket=server.nextPendingConnection();
        status->setText("客户端已连接："+clientSocket->peerAddress().toString());
        QObject::connect(clientSocket,&QTcpSocket::readyRead,clientSocket,
        [clientSocket,status,&receiveBuffer](){
            receiveBuffer.append(clientSocket->readAll());
            while(true){
                if(receiveBuffer.size()<4)return;
                QDataStream input(receiveBuffer);
                input.setByteOrder(QDataStream::BigEndian);

                quint32 payLoadsize=0;
                input>>payLoadsize;
                constexpr quint32 maxPayloadSize=10*1024*1024;
                if(maxPayloadSize<payLoadsize)
                {
                    status->setText("收到消息过大，已拒绝");
                    receiveBuffer.clear();
                    return;
                }
                if(receiveBuffer.size()<4+payLoadsize)return;
                QByteArray msg=receiveBuffer.mid(4,payLoadsize);
                receiveBuffer.remove(0,4+payLoadsize);
                if(msg.startsWith("JPEG\n"))
                {
                    QByteArray imageBytes=msg.mid(5);
                    QImage receivedImage;
                    if(receivedImage.loadFromData(imageBytes,"JPG"))
                    {
                        status->setText(status->text()+QString("\nJPEG接收成功:%1 x %2")
                        .arg(receivedImage.width())
                        .arg(receivedImage.height()));
                    }
                    else
                    {
                        status->setText(status->text()+QString("\nJPEG 解码失败"));
                    }
                }
                else {
                    status->setText(
                        status->text()+
                        "\n收到完整消息："+QString::fromUtf8(msg));
                }
            }
        }   );
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
