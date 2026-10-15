#include <QApplication>
#include <QHostAddress>

#include "appconfig.h"
#include "database/databasemanager.h"
#include "controller/servercontroller.h"
#include "network/server.h"
#include "ui/serverwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ServerWindow window;
    Server server;
    DatabaseManager database;

    // 配置先读一次。main.cpp 不解析 INI，也不做校验，那是 AppConfig 的事。
    const AppConfig config = AppConfig::load();

    // 配置有问题先把原因说出来，否则等下监听失败还得反推。
    for (const QString& text : config.notes())
    {
        window.appendStatusText(text);
    }

    ServerController controller(
        server,
        window,
        database,
        &window);

    if (!controller.initializeDatabase())
    {
        window.appendStatusText(
            "数据库不可用，服务器未启动");
    }
    else if (!controller.initializeVision())
    {
        window.appendStatusText(
            "人脸模型不可用，服务器未启动");
    }
    else
    {
        controller.loadEmployees();
        controller.loadTemplates();

        // 地址端口来自 config.ini。改这个文件重启程序就能换，不用重编译。
        //
        // 这里刻意不自己写"监听成功/失败"的提示：Server::listen() 会发
        // listeningStarted 和 listeningError 两个信号，ServerController
        // 已经把这两个信号接到状态区了，再写一遍会显示两次。
        server.listen(
            config.server().listenAddress,
            config.server().listenPort);
    }

    window.show();

    return app.exec();
}
