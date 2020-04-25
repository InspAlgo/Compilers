#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "QLRParser.h"
#include "QTableModel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<QLRParser>("QLRParser", 1, 0, "QLRParser");
    qmlRegisterType<QTableModel>("QTableModel", 1, 0, "QTableModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
