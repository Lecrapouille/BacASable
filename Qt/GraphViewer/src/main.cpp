#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

int main(int argc, char *argv[])
{
    // Configuration of the application for high resolution support
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Creation of the GUI application
    QGuiApplication app(argc, argv);

    // Configuration des métadonnées de l'application
    app.setApplicationName("GraphViewer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Lecrapouille");
    app.setOrganizationDomain("lecrappouille.com");

    // Creation of the QML engine
    QQmlApplicationEngine engine;

    // Definition of the main QML file URL
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    // Connection to check if the object was created successfully
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qDebug() << "Error : Failed to load the main QML file";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    // Loading the main QML file
    engine.load(url);

    // Checking for loading errors
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "Error : No root object loaded from the QML file";
        return -1;
    }

    qDebug() << "Application GraphViewer started successfully";
    return app.exec();
}