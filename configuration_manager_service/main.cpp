#include "ConfigurationManager.hpp"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <unistd.h>

bool startDBusDaemon() {
    // Создаем директорию для сессионной шины
    QString runtimeDir = "/run/user/" + QString::number(getuid());
    QDir().mkpath(runtimeDir);
    
    // Запускаем dbus-daemon
    QProcess dbusProcess;
    dbusProcess.start("dbus-daemon", {
        "--session",
        "--print-address=1",
        "--print-pid=1",
        "--address=unix:path=" + runtimeDir + "/bus"
    });
    
    if (!dbusProcess.waitForStarted()) {
        qCritical() << "Failed to start dbus-daemon";
        return false;
    }
    
    // Ждем инициализации
    if (!dbusProcess.waitForReadyRead()) {
        qCritical() << "dbus-daemon not ready";
        return false;
    }
    
    // Читаем адрес шины
    QString address = dbusProcess.readLine().trimmed();
    qDebug() << "DBus session address:" << address;
    
    // Устанавливаем переменную окружения
    qputenv("DBUS_SESSION_BUS_ADDRESS", address.toUtf8());
    
    // Проверяем подключение
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to D-Bus session bus";
        return false;
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    // Запускаем DBus демон
    if (!startDBusDaemon()) {
        return 1;
    }
    
    ConfigurationManager manager;
    manager.loadAllConfigurations();
    
    qInfo() << "Configuration Manager Service started";
    return app.exec();
}