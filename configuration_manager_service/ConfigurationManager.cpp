#include "ConfigurationManager.hpp"
#include <QDBusConnection>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QStandardPaths>
#include <QDBusError>

ApplicationConfiguration::ApplicationConfiguration(const QString& appName, const QString& configPath, QObject* parent)
    : QObject(parent), m_appName(appName), m_configPath(configPath)
{
    if (!loadConfiguration()) {
        qWarning() << "Failed to load configuration for" << appName;
    }
    
    // Настройка отслеживания изменений файла
    m_watcher.addPath(configPath);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ApplicationConfiguration::fileChanged);
    
    // Регистрация DBus объекта
    QString path = QString("/com/system/configurationManager/Application/%1").arg(appName);
    if (!QDBusConnection::sessionBus().registerObject(path, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qWarning() << "Failed to register DBus object for" << appName << ":" << QDBusConnection::sessionBus().lastError().message();
    }
}

QVariantMap ApplicationConfiguration::GetConfiguration() {
    QVariantMap result;
    for (auto it = m_config.begin(); it != m_config.end(); ++it) {
        result[it.key()] = it.value().toVariant();
    }
    return result;
}

void ApplicationConfiguration::ChangeConfiguration(const QString& key, const QDBusVariant& value) {
    if (!m_config.contains(key)) {
        sendErrorReply(QDBusError::InvalidArgs, QString("Key '%1' not found").arg(key));
        return;
    }
    
    m_config[key] = QJsonValue::fromVariant(value.variant());
    if (!saveConfiguration()) {
        sendErrorReply(QDBusError::Failed, "Failed to save configuration");
        return;
    }
    
    // Перезагружаем конфигурацию для отправки сигнала
    if (loadConfiguration()) {
        Q_EMIT configurationChanged(GetConfiguration());
    }
}

bool ApplicationConfiguration::loadConfiguration() {
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << m_configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON format in:" << m_configPath;
        return false;
    }
    
    m_config = doc.object();
    m_watcher.addPath(m_configPath); // Повторно добавляем файл после изменения
    return true;
}

bool ApplicationConfiguration::saveConfiguration() {
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to write config file:" << m_configPath;
        return false;
    }
    
    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    file.close();
    return true;
}

void ApplicationConfiguration::fileChanged(const QString& path) {
    if (path != m_configPath) return;
    
    if (loadConfiguration()) {
        Q_EMIT configurationChanged(GetConfiguration());
    }
}

ConfigurationManager::ConfigurationManager(QObject* parent) : QObject(parent) {}

void ConfigurationManager::loadAllConfigurations() {
    QString configDir = QDir::homePath() + "/com.system.configurationManager";
    QDir dir(configDir);
    
    if (!dir.exists()) {
        qInfo() << "Config directory does not exist, creating:" << configDir;
        dir.mkpath(".");
    }
    
    // Регистрация сервиса в DBus
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService("com.system.configurationManager")) {
        qCritical() << "Failed to register DBus service:" << bus.lastError().message();
    }
    
    // Загрузка всех конфигураций
    for (const QFileInfo& file : dir.entryInfoList(QStringList() << "*.json", QDir::Files)) {
        QString appName = file.baseName();
        if (!m_configurations.contains(appName)) {
            auto config = new ApplicationConfiguration(appName, file.absoluteFilePath(), this);
            m_configurations.insert(appName, config);
            qInfo() << "Loaded configuration for:" << appName;
        }
    }
}