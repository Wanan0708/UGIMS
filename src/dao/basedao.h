#ifndef BASEDAO_H
#define BASEDAO_H

#include <QString>
#include <QVariantMap>
#include <QSqlQuery>
#include <QVector>
#include "core/database/databasemanager.h"

/**
 * @brief DAO基类
 * 提供通用的数据库操作方法
 */
template <typename T>
class BaseDAO
{
public:
    BaseDAO(const QString &tableName)
        : m_tableName(tableName)
    {
    }

    virtual ~BaseDAO() {}

    // 纯虚函数，由子类实现
    virtual T fromQuery(QSqlQuery &query) = 0;
    virtual QVariantMap toVariantMap(const T &entity) = 0;

    // 根据ID查找
    T findById(int id)
    {
        QString sql = QString("SELECT * FROM %1 WHERE id = :id").arg(m_tableName);
        QVariantMap params;
        params[":id"] = id;

        QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
        if (query.next()) {
            return fromQuery(query);
        }
        return T();
    }

    // 查找所有记录
    QVector<T> findAll(int limit = 1000, int offset = 0)
    {
        QString sql = QString("SELECT * FROM %1 LIMIT %2 OFFSET %3")
                          .arg(m_tableName)
                          .arg(limit)
                          .arg(offset);

        QVector<T> results;
        QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
        while (query.next()) {
            results.append(fromQuery(query));
        }
        return results;
    }

    // 统计记录数
    int count(const QString &whereClause = QString())
    {
        QString sql = QString("SELECT COUNT(*) FROM %1").arg(m_tableName);
        if (!whereClause.isEmpty()) {
            sql += " WHERE " + whereClause;
        }

        QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
        if (query.next()) {
            return query.value(0).toInt();
        }
        return 0;
    }

    // 插入记录
    bool insert(const T &entity)
    {
        QVariantMap data = toVariantMap(entity);
        QStringList columns;
        QStringList placeholders;

        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            columns.append(it.key());
            placeholders.append(":" + it.key());
        }

        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                          .arg(m_tableName)
                          .arg(columns.join(", "))
                          .arg(placeholders.join(", "));

        return DatabaseManager::instance().executeCommand(sql, data);
    }

    // 更新记录
    bool update(const T &entity, int id)
    {
        QVariantMap data = toVariantMap(entity);
        QStringList setParts;

        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            setParts.append(QString("%1 = :%1").arg(it.key()));
        }

        QString sql = QString("UPDATE %1 SET %2 WHERE id = :id")
                          .arg(m_tableName)
                          .arg(setParts.join(", "));

        data[":id"] = id;
        return DatabaseManager::instance().executeCommand(sql, data);
    }

    // 删除记录
    bool deleteById(int id)
    {
        QString sql = QString("DELETE FROM %1 WHERE id = :id").arg(m_tableName);
        QVariantMap params;
        params[":id"] = id;

        return DatabaseManager::instance().executeCommand(sql, params);
    }

protected:
    QString m_tableName;
};

#endif // BASEDAO_H

