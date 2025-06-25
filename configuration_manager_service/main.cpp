#include "ConfigurationManager.hpp"
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char* argv[]) {
    if (qEnvironmentVariableIsEmpty("DBUS_SESSION_BUS_ADDRESS")) {
        qputenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/run/user/1000/bus");
    }
    
    QCoreApplication app(argc, argv);
    
    ConfigurationManager manager;
    manager.loadAllConfigurations();
    
    qInfo() << "Configuration Manager Service started";
    
    return app.exec();
}