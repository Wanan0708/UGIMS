#include "core/common/config.h"
#include "core/common/logger.h"
#include <QFileInfo>

Config& Config::instance()
{
    static Config instance;
    return instance;
}

Config::Config()
    : m_appSettings(nullptr)
    , m_dbSettings(nullptr)
{
}

Config::~Config()
{
    delete m_appSettings;
    delete m_dbSettings;
}

void Config::initialize(const QString &configPath)
{
    QMutexLocker locker(&m_mutex);

    if (m_appSettings) {
        delete m_appSettings;
    }

    QFileInfo fileInfo(configPath);
    if (!fileInfo.exists()) {
        LOG_WARNING(QString("Config file not found: %1, using defaults").arg(configPath));
    }

    m_appSettings = new QSettings(configPath, QSettings::IniFormat);
    LOG_INFO(QString("Application config loaded: %1").arg(configPath));
}

bool Config::loadDatabaseConfig(const QString &dbConfigPath)
{
    QMutexLocker locker(&m_mutex);

    if (m_dbSettings) {
        delete m_dbSettings;
    }

    QFileInfo fileInfo(dbConfigPath);
    if (!fileInfo.exists()) {
        LOG_ERROR(QString("Database config file not found: %1").arg(dbConfigPath));
        return false;
    }

    m_dbSettings = new QSettings(dbConfigPath, QSettings::IniFormat);
    LOG_INFO(QString("Database config loaded: %1").arg(dbConfigPath));
    return true;
}

QString Config::getString(const QString &key, const QString &defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_appSettings) return defaultValue;
    return m_appSettings->value(key, defaultValue).toString();
}

int Config::getInt(const QString &key, int defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_appSettings) return defaultValue;
    return m_appSettings->value(key, defaultValue).toInt();
}

bool Config::getBool(const QString &key, bool defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_appSettings) return defaultValue;
    return m_appSettings->value(key, defaultValue).toBool();
}

double Config::getDouble(const QString &key, double defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_appSettings) return defaultValue;
    return m_appSettings->value(key, defaultValue).toDouble();
}

QString Config::getDatabaseType() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return "postgresql";
    return m_dbSettings->value("database/type", "postgresql").toString();
}

QString Config::getDatabaseHost() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return "localhost";
    return m_dbSettings->value("database/host", "localhost").toString();
}

int Config::getDatabasePort() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return 5432;
    return m_dbSettings->value("database/port", 5432).toInt();
}

QString Config::getDatabaseName() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return "ugims";
    return m_dbSettings->value("database/dbname", "ugims").toString();
}

QString Config::getDatabaseUsername() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return "postgres";
    return m_dbSettings->value("database/username", "postgres").toString();
}

QString Config::getDatabasePassword() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return "";
    return m_dbSettings->value("database/password", "").toString();
}

int Config::getMaxConnections() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return 10;
    return m_dbSettings->value("database/max_connections", 10).toInt();
}

int Config::getMinConnections() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_dbSettings) return 2;
    return m_dbSettings->value("database/min_connections", 2).toInt();
}

void Config::setValue(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&m_mutex);
    if (m_appSettings) {
        m_appSettings->setValue(key, value);
    }
}

void Config::save()
{
    QMutexLocker locker(&m_mutex);
    if (m_appSettings) {
        m_appSettings->sync();
    }
}

