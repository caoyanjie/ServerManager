#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "SystemPath.h"
#include "UdpTransmission.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    RakNetServer::SystemPath::initServerCacheDir(argv[0]);
    qmlRegisterType<RakNetServer::UdpTransmission>("QtCPlusPlus.UdpTransmission", 2, 0, "UdpTransmission");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/QML/main.qml")));

    return app.exec();
}
