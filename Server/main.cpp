#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "SystemPath.h"
#include "UdpTransmission.h"
#include "ImageProvider.h"
#include "ImageTransfer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    RakNetServer::SystemPath::initServerCacheDir(argv[0]);
    qmlRegisterType<RakNetServer::UdpTransmission>("QtCPlusPlus.UdpTransmission", 2, 0, "UdpTransmission");
    qmlRegisterType<ImageTransfer>("QtCPlusPlus.ImageTransfer", 2, 0, "ImageTransfer");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("ImageProvider"), ImageProvider::GetInstance());
    engine.load(QUrl(QStringLiteral("qrc:/QML/main.qml")));

    return app.exec();
}
