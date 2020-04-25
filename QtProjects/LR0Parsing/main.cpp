#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "lr0parser.h"
#include "TableModel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<LR0Parser>("LR0Parser", 1, 0, "LR0Parser");
    qmlRegisterType<TableModel>("TableModel", 1, 0, "TableModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
