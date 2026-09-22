#include <QApplication>
#include <QHostAddress>

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
    else
    {
        controller.loadEmployees();

        server.listen(
            QHostAddress::LocalHost,
            45454);
    }

    window.show();

    return app.exec();
}
