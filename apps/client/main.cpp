#include <QApplication>
#include <QLabel>
#include <QMainWindow>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Face Attendance Client");
    window.setMinimumSize(960, 640);

    QLabel* status = new QLabel("First qt test", &window);
    status->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(status);
    window.show();

    return app.exec();
}
