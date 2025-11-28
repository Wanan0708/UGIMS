#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QMutex>
#include <QVector>
#include <QVariantMap>

/**
 * @brief 数据库管理器
 * 单例模式，管理PostgreSQL数据库连接
 * 使用Qt的QSqlDatabase实现
 */
class DatabaseManager
{
public:
    // 获取单例实例
    static DatabaseManager& instance();

    // 初始化数据库连接
    bool initialize();

    // 连接数据库
    bool connect();

    // 断开连接
    void disconnect();

    // 检查连接状态
    bool isConnected() const;

    // 执行SQL查询（SELECT）
    QSqlQuery executeQuery(const QString &sql, const QVariantMap &params = QVariantMap());

    // 执行SQL命令（INSERT, UPDATE, DELETE）
    bool executeCommand(const QString &sql, const QVariantMap &params = QVariantMap());

    // 开始事务
    bool beginTransaction();

    // 提交事务
    bool commit();

    // 回滚事务
    bool rollback();

    // 获取最后的错误信息
    QString lastError() const;

    // 获取数据库实例（用于高级操作）
    QSqlDatabase database();

    // 禁用拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase m_database;
    mutable QMutex m_mutex;
    QString m_lastError;
    bool m_initialized;

    // 绑定参数到查询
    void bindParameters(QSqlQuery &query, const QVariantMap &params);
};

#endif // DATABASEMANAGER_H

