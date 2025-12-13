#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include "core/models/user.h"

/**
 * @brief 会话管理器
 * 管理用户登录状态和当前会话
 */
class SessionManager : public QObject
{
    Q_OBJECT

public:
    static SessionManager& instance();
    
    // 登录
    bool login(const QString &username, const QString &password);
    
    // 登出
    void logout();
    
    // 检查是否已登录
    bool isLoggedIn() const { return m_currentUser.isValid(); }
    
    // 获取当前用户
    User currentUser() const { return m_currentUser; }
    
    // 获取当前用户名
    QString currentUsername() const { return m_currentUser.username(); }
    
    // 获取当前用户角色
    QString currentRole() const { return m_currentUser.role(); }
    
    // 权限检查
    bool hasPermission(const QString &permission) const;

signals:
    void userLoggedIn(const User &user);
    void userLoggedOut();

private:
    SessionManager(QObject *parent = nullptr);
    ~SessionManager();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    
    User m_currentUser;
};

#endif // SESSIONMANAGER_H

