#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "ll1parsingtoqml.h"
#include "tablemodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    qmlRegisterType<LL1ParsingToQML>("LL1ParsingToQML", 1, 0, "LL1ParsingToQML");
    qmlRegisterType<TableModel>("TableModel", 1, 0, "TableModel");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
