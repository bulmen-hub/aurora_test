#include "ConfigurationManager.hpp"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <QStandardPaths>
#include <QThread>
#include <cstring>

QString s_runtimePath;

void createRuntimeDir() {
    s_runtimePath = "/run/user/" + QString::number(getuid());
    QDir dir(s_runtimePath);
    
    if (!dir.exists()) {
        qDebug() << "Creating runtime directory:" << s_runtimePath;
        if (mkdir(s_runtimePath.toUtf8().constData(), 0700) != 0) {
            qCritical() << "Failed to create runtime directory:" << s_runtimePath 
                        << "Error:" << strerror(errno);
            
            // Попробуем создать в домашней директории
            s_runtimePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) 
                         + "/.dbus-runtime";
            qDebug() << "Trying alternative location:" << s_runtimePath;
            QDir homeDir(s_runtimePath);
            if (!homeDir.exists() && !homeDir.mkpath(".")) {
                qCritical() << "Failed to create alternative runtime directory";
                return;
            }
        }
    }
}

bool startDBusDaemon() {
    createRuntimeDir();
    
    QString busPath = s_runtimePath + "/bus";
    
    // Проверяем, не запущен ли уже демон
    if (QFile::exists(busPath)) {
        qDebug() << "DBus session bus already exists at:" << busPath;
        qputenv("DBUS_SESSION_BUS_ADDRESS", ("unix:path=" + busPath).toUtf8());
        return true;
    }
    
    QProcess dbusProcess;
    dbusProcess.setProcessChannelMode(QProcess::MergedChannels);
    dbusProcess.start("dbus-daemon", {
        "--session",
        "--print-address=1",
        "--print-pid=1",
        "--address=unix:path=" + busPath
    });
    
    if (!dbusProcess.waitForStarted(5000)) {
        qCritical() << "Failed to start dbus-daemon:" << dbusProcess.errorString();
        return false;
    }
    
    // Даем демону время на инициализацию
    if (!dbusProcess.waitForReadyRead(5000)) {
        qCritical() << "dbus-daemon not ready:" << dbusProcess.errorString();
        return false;
    }
    
    // Читаем адрес шины
    QString address = QString::fromLocal8Bit(dbusProcess.readLine()).trimmed();
    if (address.isEmpty()) {
        qCritical() << "Failed to get DBus address";
        return false;
    }
    
    qDebug() << "DBus session address:" << address;
    qputenv("DBUS_SESSION_BUS_ADDRESS", address.toUtf8());
    
    // Проверяем подключение
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus";
        return false;
    }
    
    // Ждем окончательной инициализации
    QThread::sleep(1);
    
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
