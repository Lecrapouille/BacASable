#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include "mainwindow.h"
#include "petrinetmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Enregistrer le type Arc avec QML
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

    // Vérifier si le chargement a réussi
    if (window->quickWidget()->status() == QQuickWidget::Error) {
        qDebug() << "Erreurs lors du chargement du QML:";
        for (const QQmlError &error : window->quickWidget()->errors()) {
            qDebug() << error.toString();
        }
    }

    window->show();

    return app.exec();
}