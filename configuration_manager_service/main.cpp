#include "ConfigurationManager.hpp"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    // Проверяем подключение к DBus
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus:"
                    << QDBusConnection::sessionBus().lastError().message();
        return 1;
    }
    
    ConfigurationManager manager;
    manager.loadAllConfigurations();
    
    qInfo() << "Configuration Manager Service started";
    return app.exec();
}