#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "ViewModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Créer le ViewModel et l'exposer à QML
    ImageViewModel *viewModel = new ImageViewModel(&app);

    // Ajouter quelques images initiales via C++
    viewModel->addImage("PNG", 1, "Image 1");
    viewModel->addImage("JPG", 2, "Image 2");
    viewModel->addImage("GIF", 3, "Image 3");

    QQmlApplicationEngine engine;

    // Exposer le ViewModel à QML
    engine.rootContext()->setContextProperty("imageViewModel", viewModel);

    // Charger le fichier QML principal
    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    // Test : Ajouter/supprimer des images via C++ après démarrage
    QTimer *timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, [viewModel]() {
        static int counter = 0;
        counter++;

        if (counter == 30) { // Après 3 secondes
            viewModel->addImage("BMP", 4, "Image 4 (from C++)");
        } else if (counter == 60) { // Après 6 secondes
            if (viewModel->getImageCount() > 0) {
                viewModel->removeImage(0); // Supprimer le premier
            }
        } else if (counter == 90) { // Après 9 secondes
            viewModel->addImage("TIFF", 5, "Image 5 (from C++)");
        }
    });
    timer->start(100); // Timer toutes les 100ms

    return app.exec();
}
