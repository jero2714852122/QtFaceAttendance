#include <QApplication>
#include <QLabel>
#include <QMainWindow>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Face Attendance Server");
    window.setMinimumSize(960, 640);

    auto* status = new QLabel("Server environment ready", &window);
    status->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(status);
    window.show();

    return app.exec();
}
