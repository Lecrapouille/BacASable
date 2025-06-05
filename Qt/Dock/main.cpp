#include "MainWindow.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Qt6 Display App");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MonOrganisation");

    MainWindow window;
    window.show();

    return app.exec();
}