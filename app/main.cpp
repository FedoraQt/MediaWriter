#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "drivemanager.h"
#include "releasemanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("drives", new DriveManager());
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
