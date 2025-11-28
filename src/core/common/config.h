#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>
#include <QVariant>
#include <QMutex>

/**
 * @brief 配置管理类
 * 单例模式，负责读取和管理所有配置文件
 */
class Config
{
public:
    // 获取单例实例
    static Config& instance();

    // 初始化配置
    void initialize(const QString &configPath = "config/app.ini");

    // 加载数据库配置
    bool loadDatabaseConfig(const QString &dbConfigPath = "config/database.ini");

    // 获取配置值
    QString getString(const QString &key, const QString &defaultValue = QString()) const;
    int getInt(const QString &key, int defaultValue = 0) const;
    bool getBool(const QString &key, bool defaultValue = false) const;
    double getDouble(const QString &key, double defaultValue = 0.0) const;

    // 数据库配置获取
    QString getDatabaseType() const;
    QString getDatabaseHost() const;
    int getDatabasePort() const;
    QString getDatabaseName() const;
    QString getDatabaseUsername() const;
    QString getDatabasePassword() const;
    int getMaxConnections() const;
    int getMinConnections() const;

    // 设置配置值
    void setValue(const QString &key, const QVariant &value);

    // 保存配置
    void save();

    // 禁用拷贝
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    Config();
    ~Config();

    QSettings *m_appSettings;
    QSettings *m_dbSettings;
    mutable QMutex m_mutex;
};

#endif // CONFIG_H

