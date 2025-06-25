#pragma once

#include <QObject>
#include <QMap>
#include <QDBusVariant>
#include <QString>
#include <QDBusContext>
#include <QJsonObject>
#include <QFileSystemWatcher>

class ApplicationConfiguration : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.system.configurationManager.Application.Configuration")

public:
    ApplicationConfiguration(const QString& appName, const QString& configPath, QObject* parent = nullptr);
    
public Q_SLOTS:
    QVariantMap GetConfiguration();
    void ChangeConfiguration(const QString& key, const QDBusVariant& value);

Q_SIGNALS:
    void configurationChanged(const QVariantMap& configuration);

private:
    bool loadConfiguration();
    bool saveConfiguration();
    void fileChanged(const QString& path);

    QString m_appName;
    QString m_configPath;
    QJsonObject m_config;
    QFileSystemWatcher m_watcher;
};

class ConfigurationManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigurationManager(QObject* parent = nullptr);
    void loadAllConfigurations();

private:
    QMap<QString, ApplicationConfiguration*> m_configurations;
};