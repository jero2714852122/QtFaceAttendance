#include <QApplication>
#include <QHostAddress>

#include "controller/servercontroller.h"
#include "network/server.h"
#include "ui/serverwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ServerWindow window;
    Server server;

    ServerController controller(
        server,
        window,
        &window);

    server.listen(
        QHostAddress::LocalHost,
        45454);

    window.show();

    return app.exec();
}