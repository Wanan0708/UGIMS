#include "userdao.h"
#include "core/database/databasemanager.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

UserDAO::UserDAO()
    : BaseDAO<User>("users")
{
}

User UserDAO::fromQuery(QSqlQuery &query)
{
    User user;
    
    user.setId(query.value("id").toInt());
    user.setUsername(query.value("username").toString());
    user.setPasswordHash(query.value("password_hash").toString());
    user.setRealName(query.value("real_name").toString());
    user.setEmail(query.value("email").toString());
    user.setPhone(query.value("phone").toString());
    user.setRole(query.value("role").toString());
    
    // 解析权限数组
    QString permissionsStr = query.value("permissions").toString();
    if (!permissionsStr.isEmpty()) {
        // PostgreSQL数组格式: {permission1,permission2,...}
        permissionsStr = permissionsStr.mid(1, permissionsStr.length() - 2); // 移除 { }
        QStringList permissions = permissionsStr.split(',', Qt::SkipEmptyParts);
        user.setPermissions(permissions);
    }
    
    user.setDepartment(query.value("department").toString());
    user.setOrganization(query.value("organization").toString());
    user.setStatus(query.value("status").toString());
    user.setLastLogin(query.value("last_login").toDateTime());
    user.setCreatedAt(query.value("created_at").toDateTime());
    user.setUpdatedAt(query.value("updated_at").toDateTime());
    
    return user;
}

QVariantMap UserDAO::toVariantMap(const User &user)
{
    QVariantMap data;
    
    if (user.id() > 0) {
        data["id"] = user.id();
    }
    data["username"] = user.username();
    data["password_hash"] = user.passwordHash();
    data["real_name"] = user.realName();
    data["email"] = user.email();
    data["phone"] = user.phone();
    data["role"] = user.role();
    
    // 将权限列表转换为PostgreSQL数组格式
    QStringList perms = user.permissions();
    if (!perms.isEmpty()) {
        QString permsStr = "{" + perms.join(",") + "}";
        data["permissions"] = permsStr;
    } else {
        data["permissions"] = QVariant();  // NULL
    }
    
    data["department"] = user.department();
    data["organization"] = user.organization();
    data["status"] = user.status();
    
    if (user.lastLogin().isValid()) {
        data["last_login"] = user.lastLogin();
    }
    if (user.createdAt().isValid()) {
        data["created_at"] = user.createdAt();
    }
    data["updated_at"] = user.updatedAt();
    
    return data;
}

User UserDAO::findByUsername(const QString &username)
{
    QString sql = "SELECT * FROM users WHERE username = :username";
    QVariantMap params;
    params[":username"] = username;
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    if (query.next()) {
        return fromQuery(query);
    }
    return User();
}

bool UserDAO::verifyPassword(const QString &username, const QString &password)
{
    User user = findByUsername(username);
    if (!user.isValid()) {
        return false;
    }
    
    // 计算密码哈希
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
    );
    QString passwordHash = hash.toHex();
    
    // 比较哈希值
    return user.passwordHash() == passwordHash;
}

bool UserDAO::updatePassword(int userId, const QString &newPasswordHash)
{
    QString sql = "UPDATE users SET password_hash = :password_hash, updated_at = CURRENT_TIMESTAMP WHERE id = :id";
    QVariantMap params;
    params[":password_hash"] = newPasswordHash;
    params[":id"] = userId;
    
    return DatabaseManager::instance().executeCommand(sql, params);
}

bool UserDAO::updateLastLogin(int userId)
{
    QString sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = :id";
    QVariantMap params;
    params[":id"] = userId;
    
    return DatabaseManager::instance().executeCommand(sql, params);
}

QVector<User> UserDAO::findByRole(const QString &role)
{
    QString sql = "SELECT * FROM users WHERE role = :role";
    QVariantMap params;
    params[":role"] = role;
    
    QVector<User> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<User> UserDAO::findByStatus(const QString &status)
{
    QString sql = "SELECT * FROM users WHERE status = :status";
    QVariantMap params;
    params[":status"] = status;
    
    QVector<User> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

bool UserDAO::usernameExists(const QString &username, int excludeId)
{
    QString sql = "SELECT COUNT(*) FROM users WHERE username = :username";
    if (excludeId > 0) {
        sql += " AND id != :exclude_id";
    }
    
    QVariantMap params;
    params[":username"] = username;
    if (excludeId > 0) {
        params[":exclude_id"] = excludeId;
    }
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool UserDAO::createUser(const User &user, const QString &plainPassword)
{
    // 计算密码哈希
    QByteArray hash = QCryptographicHash::hash(
        plainPassword.toUtf8(),
        QCryptographicHash::Sha256
    );
    QString passwordHash = hash.toHex();
    
    // 创建用户对象（带密码哈希）
    User newUser = user;
    newUser.setPasswordHash(passwordHash);
    newUser.setCreatedAt(QDateTime::currentDateTime());
    newUser.setUpdatedAt(QDateTime::currentDateTime());
    
    return insert(newUser);
}

bool UserDAO::updateUser(const User &user)
{
    User updatedUser = user;
    updatedUser.setUpdatedAt(QDateTime::currentDateTime());
    
    return update(updatedUser, user.id());
}

