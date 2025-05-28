#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include "mainwindow.h"
#include "petrinetmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Configuration of the application metadata
    app.setApplicationName("PetriNetViewer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Lecrapouille");
    app.setOrganizationDomain("lecrappouille.com");

    // Register the Arc type with QML
    qRegisterMetaType<Arc>();

    // Create the model
    PetriNetModel* model = new PetriNetModel(&app);

    // Create window
    MainWindow* window = new MainWindow();
    window->setPetriNetModel(model);

    // Register the model with QML
    window->quickWidget()->engine()->rootContext()->setContextProperty("petriNetModel", model);

    // Set the QML source after setting context property
    QUrl source("qrc:/main.qml");
    window->quickWidget()->setSource(source);

    // Check if the loading was successful
    if (window->quickWidget()->status() == QQuickWidget::Error) {
        qDebug() << "Errors while loading the QML:";
        for (const QQmlError &error : window->quickWidget()->errors()) {
            qDebug() << error.toString();
        }
    }

    window->show();

    return app.exec();
}