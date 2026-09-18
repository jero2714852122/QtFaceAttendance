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

    controller.initializeDatabase();

    server.listen(
        QHostAddress::LocalHost,
        45454);

    window.show();

    return app.exec();
}
