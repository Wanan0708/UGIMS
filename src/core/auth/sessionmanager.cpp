#include "sessionmanager.h"
#include "dao/userdao.h"
#include "core/common/logger.h"
#include <QDebug>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
}

SessionManager::~SessionManager()
{
}

SessionManager& SessionManager::instance()
{
    static SessionManager instance;
    return instance;
}

bool SessionManager::login(const QString &username, const QString &password)
{
    UserDAO dao;
    
    // 验证密码
    if (!dao.verifyPassword(username, password)) {
        LOG_WARNING(QString("Login failed for user: %1").arg(username));
        return false;
    }
    
    // 获取用户信息
    User user = dao.findByUsername(username);
    if (!user.isValid()) {
        LOG_WARNING(QString("User not found: %1").arg(username));
        return false;
    }
    
    // 检查账户状态
    if (!user.isActive()) {
        LOG_WARNING(QString("User account is not active: %1").arg(username));
        return false;
    }
    
    // 更新最后登录时间
    dao.updateLastLogin(user.id());
    
    // 设置当前用户
    m_currentUser = user;
    
    LOG_INFO(QString("User logged in: %1 (role: %2)").arg(username).arg(user.role()));
    emit userLoggedIn(user);
    
    return true;
}

void SessionManager::logout()
{
    if (m_currentUser.isValid()) {
        QString username = m_currentUser.username();
        m_currentUser = User();
        LOG_INFO(QString("User logged out: %1").arg(username));
        emit userLoggedOut();
    }
}

bool SessionManager::hasPermission(const QString &permission) const
{
    if (!isLoggedIn()) {
        return false;
    }
    
    return m_currentUser.hasPermission(permission);
}

