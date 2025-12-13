#ifndef USERDAO_H
#define USERDAO_H

#include "basedao.h"
#include "core/models/user.h"
#include <QString>
#include <QVector>

/**
 * @brief 用户数据访问对象
 * 提供用户的CRUD操作
 */
class UserDAO : public BaseDAO<User>
{
public:
    UserDAO();
    
    // 从查询结果构建User对象
    User fromQuery(QSqlQuery &query) override;
    
    // 将User对象转换为QVariantMap
    QVariantMap toVariantMap(const User &user) override;
    
    // 根据用户名查找用户
    User findByUsername(const QString &username);
    
    // 验证用户密码
    bool verifyPassword(const QString &username, const QString &password);
    
    // 更新用户密码
    bool updatePassword(int userId, const QString &newPasswordHash);
    
    // 更新最后登录时间
    bool updateLastLogin(int userId);
    
    // 根据角色查找用户
    QVector<User> findByRole(const QString &role);
    
    // 根据状态查找用户
    QVector<User> findByStatus(const QString &status);
    
    // 检查用户名是否存在
    bool usernameExists(const QString &username, int excludeId = 0);
    
    // 创建用户（带密码加密）
    bool createUser(const User &user, const QString &plainPassword);
    
    // 更新用户（不更新密码）
    bool updateUser(const User &user);
};

#endif // USERDAO_H

