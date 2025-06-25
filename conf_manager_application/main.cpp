#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <QDebug>

class ConfigClient : public QObject {
    Q_OBJECT
public:
    ConfigClient(QObject* parent = nullptr) : QObject(parent) {
        iface = new QDBusInterface(
            "com.system.configurationManager",
            "/com/system/configurationManager/Application/confManagerApplication1",
            "com.system.configurationManager.Application.Configuration",
            QDBusConnection::sessionBus(),
            this
        );

        if (!iface->isValid()) {
            qWarning() << "DBus interface error:" << QDBusConnection::sessionBus().lastError().message();
            return;
        }

        // Подписка на сигнал
        if (!QDBusConnection::sessionBus().connect(
            iface->service(),
            iface->path(),
            iface->interface(),
            "configurationChanged",
            this,
            SLOT(onConfigurationChanged(QVariantMap))
        )) {
            qWarning() << "Failed to connect to configurationChanged signal";
        }

        loadConfiguration();
    }

public slots:
    void onConfigurationChanged(const QVariantMap& config) {
        qInfo() << "Configuration changed, reloading...";
        applyConfiguration(config);
    }

    void onTimeout() {
        qInfo() << m_timeoutPhrase;
    }

private:
    void loadConfiguration() {
        QDBusReply<QVariantMap> reply = iface->call("GetConfiguration");
        if (!reply.isValid()) {
            qWarning() << "DBus call error:" << reply.error().message();
            return;
        }
        applyConfiguration(reply.value());
    }

    void applyConfiguration(const QVariantMap& config) {
        if (timer) {
            timer->stop();
            delete timer;
        }

        m_timeout = config.value("Timeout", 1000).toUInt();
        m_timeoutPhrase = config.value("TimeoutPhrase", "Default timeout message").toString();

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ConfigClient::onTimeout);
        timer->start(m_timeout);

        qInfo() << "New configuration applied:"
                << "Timeout =" << m_timeout << "ms,"
                << "Phrase =" << m_timeoutPhrase;
    }

    QDBusInterface* iface = nullptr;
    QTimer* timer = nullptr;
    uint m_timeout = 1000;
    QString m_timeoutPhrase;
};

#include "main.moc"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    // Проверяем подключение к DBus
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus:"
                    << QDBusConnection::sessionBus().lastError().message();
        return 1;
    }
    
    qInfo() << "Configuration Client Application started";
    ConfigClient client;
    return app.exec();
}