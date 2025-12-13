#include "core/database/databasemanager.h"
#include "core/common/logger.h"
#include "core/common/config.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
    : m_initialized(false)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

bool DatabaseManager::initialize()
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        LOG_WARNING("DatabaseManager already initialized");
        return true;
    }

    // 从配置读取数据库信息（配置应该已经在外部加载）
    Config &config = Config::instance();
    QString dbType = config.getDatabaseType();

    // 创建数据库连接
    if (dbType == "postgresql") {
        // 检查PostgreSQL驱动是否可用
        if (!QSqlDatabase::isDriverAvailable("QPSQL")) {
            m_lastError = "PostgreSQL driver (QPSQL) is not available. "
                         "Please ensure Qt PostgreSQL plugin is installed. "
                         "Available drivers: " + QSqlDatabase::drivers().join(", ");
            LOG_ERROR(m_lastError);
            qDebug() << "[DB] Available drivers:" << QSqlDatabase::drivers();
            return false;
        }
        m_database = QSqlDatabase::addDatabase("QPSQL", "ugims_connection");
    } else if (dbType == "sqlite") {
        m_database = QSqlDatabase::addDatabase("QSQLITE", "ugims_connection");
    } else {
        m_lastError = QString("Unsupported database type: %1").arg(dbType);
        LOG_ERROR(m_lastError);
        return false;
    }

    // 设置数据库参数
    if (dbType == "postgresql") {
        QString host = config.getDatabaseHost();
        int port = config.getDatabasePort();
        QString dbname = config.getDatabaseName();
        QString username = config.getDatabaseUsername();
        QString password = config.getDatabasePassword();
        
        m_database.setHostName(host);
        m_database.setPort(port);
        m_database.setDatabaseName(dbname);
        m_database.setUserName(username);
        m_database.setPassword(password);

        LOG_INFO(QString("Database configured: PostgreSQL at %1:%2/%3 (user: %4)")
                     .arg(host)
                     .arg(port)
                     .arg(dbname)
                     .arg(username));
        
        qDebug() << "[DB] Type:" << dbType;
        qDebug() << "[DB] Host:" << host;
        qDebug() << "[DB] Port:" << port;
        qDebug() << "[DB] Database:" << dbname;
        qDebug() << "[DB] Username:" << username;
        qDebug() << "[DB] Password:" << (password.isEmpty() ? "empty" : "***");
        
    } else if (dbType == "sqlite") {
        m_database.setDatabaseName(config.getString("database/sqlite_path", "data/ugims.db"));
        LOG_INFO(QString("Database configured: SQLite at %1")
                     .arg(m_database.databaseName()));
    }

    m_initialized = true;
    return true;
}

bool DatabaseManager::connect()
{
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        m_lastError = "DatabaseManager not initialized";
        LOG_ERROR(m_lastError);
        return false;
    }

    if (m_database.isOpen()) {
        LOG_WARNING("Database already connected");
        return true;
    }

    // 设置连接选项（添加超时）
    if (m_database.driverName() == "QPSQL") {
        // PostgreSQL 连接超时设置
        m_database.setConnectOptions("connect_timeout=5");  // 5秒超时
        LOG_INFO("Set PostgreSQL connection timeout to 5 seconds");
    }

    LOG_INFO("Attempting to connect to database...");
    
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        LOG_ERROR(QString("Failed to connect to database: %1").arg(m_lastError));
        
        // 提供更友好的错误信息
        if (m_lastError.contains("timeout", Qt::CaseInsensitive)) {
            m_lastError = "数据库连接超时，请检查PostgreSQL是否运行";
        } else if (m_lastError.contains("password", Qt::CaseInsensitive)) {
            m_lastError = "数据库密码错误，请检查config/database.ini";
        } else if (m_lastError.contains("database", Qt::CaseInsensitive) && 
                   m_lastError.contains("does not exist", Qt::CaseInsensitive)) {
            m_lastError = "数据库不存在，请先创建ugims数据库";
        }
        
        return false;
    }

    LOG_INFO("Successfully connected to database");

    // 对于PostgreSQL，测试PostGIS扩展是否可用
    if (m_database.driverName() == "QPSQL") {
        QSqlQuery query(m_database);
        query.setForwardOnly(true);  // 优化性能
        
        if (query.exec("SELECT PostGIS_Version()")) {
            if (query.next()) {
                QString version = query.value(0).toString();
                LOG_INFO(QString("PostGIS version: %1").arg(version));
            }
        } else {
            LOG_WARNING("PostGIS extension not available - spatial queries may not work");
            LOG_WARNING("Run: CREATE EXTENSION IF NOT EXISTS postgis;");
        }
    }

    return true;
}

void DatabaseManager::disconnect()
{
    QMutexLocker locker(&m_mutex);

    if (m_database.isOpen()) {
        m_database.close();
        LOG_INFO("Database disconnected");
    }
}

bool DatabaseManager::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_database.isOpen();
}

QSqlQuery DatabaseManager::executeQuery(const QString &sql, const QVariantMap &params)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    
    // 如果没有参数，直接执行（避免QPSQL驱动的prepare问题）
    if (params.isEmpty()) {
        if (!query.exec(sql)) {
            m_lastError = query.lastError().text();
            LOG_ERROR(QString("Query failed: %1\nSQL: %2").arg(m_lastError, sql));
        } else {
            LOG_DEBUG(QString("Query executed: %1").arg(sql));
        }
    } else {
        query.prepare(sql);
        bindParameters(query, params);
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            LOG_ERROR(QString("Query failed: %1\nSQL: %2").arg(m_lastError, sql));
        } else {
            LOG_DEBUG(QString("Query executed: %1").arg(sql));
        }
    }

    return query;
}

bool DatabaseManager::executeCommand(const QString &sql, const QVariantMap &params)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    
    // 如果没有参数，直接执行（避免QPSQL驱动的prepare问题）
    if (params.isEmpty()) {
        if (!query.exec(sql)) {
            m_lastError = query.lastError().text();
            LOG_ERROR(QString("Command failed: %1\nSQL: %2").arg(m_lastError, sql));
            return false;
        }
    } else {
        query.prepare(sql);
        bindParameters(query, params);
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            LOG_ERROR(QString("Command failed: %1\nSQL: %2").arg(m_lastError, sql));
            return false;
        }
    }

    LOG_DEBUG(QString("Command executed: %1, affected rows: %2")
                  .arg(sql)
                  .arg(query.numRowsAffected()));
    return true;
}

bool DatabaseManager::beginTransaction()
{
    QMutexLocker locker(&m_mutex);

    if (!m_database.transaction()) {
        m_lastError = m_database.lastError().text();
        LOG_ERROR(QString("Failed to begin transaction: %1").arg(m_lastError));
        return false;
    }

    LOG_DEBUG("Transaction started");
    return true;
}

bool DatabaseManager::commit()
{
    QMutexLocker locker(&m_mutex);

    if (!m_database.commit()) {
        m_lastError = m_database.lastError().text();
        LOG_ERROR(QString("Failed to commit transaction: %1").arg(m_lastError));
        return false;
    }

    LOG_DEBUG("Transaction committed");
    return true;
}

bool DatabaseManager::rollback()
{
    QMutexLocker locker(&m_mutex);

    if (!m_database.rollback()) {
        m_lastError = m_database.lastError().text();
        LOG_ERROR(QString("Failed to rollback transaction: %1").arg(m_lastError));
        return false;
    }

    LOG_DEBUG("Transaction rolled back");
    return true;
}

QString DatabaseManager::lastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

QSqlDatabase DatabaseManager::database()
{
    return m_database;
}

void DatabaseManager::bindParameters(QSqlQuery &query, const QVariantMap &params)
{
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        // 确保占位符带冒号前缀
        QString placeholder = it.key().startsWith(':') ? it.key() : (":" + it.key());
        query.bindValue(placeholder, it.value());
    }
}

