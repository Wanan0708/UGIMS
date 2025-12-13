#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QStringList>

/**
 * @brief 用户数据模型
 * 对应数据库 users 表
 */
class User
{
public:
    // 用户角色枚举
    enum Role {
        Admin,      // 管理员
        Manager,    // 管理者
        Inspector,  // 巡检员
        Viewer      // 查看者
    };
    
    // 账户状态枚举
    enum Status {
        Active,     // 激活
        Inactive,   // 未激活
        Locked      // 锁定
    };
    
    // 静态常量
    static const QString ROLE_ADMIN;
    static const QString ROLE_MANAGER;
    static const QString ROLE_INSPECTOR;
    static const QString ROLE_VIEWER;
    
    static const QString STATUS_ACTIVE;
    static const QString STATUS_INACTIVE;
    static const QString STATUS_LOCKED;

    User();
    
    // 有效性检查
    bool isValid() const { return m_id > 0 && !m_username.isEmpty(); }
    
    // Getter
    int id() const { return m_id; }
    QString username() const { return m_username; }
    QString passwordHash() const { return m_passwordHash; }
    QString realName() const { return m_realName; }
    QString email() const { return m_email; }
    QString phone() const { return m_phone; }
    QString role() const { return m_role; }
    QStringList permissions() const { return m_permissions; }
    QString department() const { return m_department; }
    QString organization() const { return m_organization; }
    QString status() const { return m_status; }
    QDateTime lastLogin() const { return m_lastLogin; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }
    
    // 角色检查方法
    bool isAdmin() const { return m_role == ROLE_ADMIN; }
    bool isManager() const { return m_role == ROLE_MANAGER; }
    bool isInspector() const { return m_role == ROLE_INSPECTOR; }
    bool isViewer() const { return m_role == ROLE_VIEWER; }
    bool isActive() const { return m_status == STATUS_ACTIVE; }
    
    // 权限检查
    bool hasPermission(const QString &permission) const;
    
    // Setter
    void setId(int id) { m_id = id; }
    void setUsername(const QString &username) { m_username = username; }
    void setPasswordHash(const QString &hash) { m_passwordHash = hash; }
    void setRealName(const QString &name) { m_realName = name; }
    void setEmail(const QString &email) { m_email = email; }
    void setPhone(const QString &phone) { m_phone = phone; }
    void setRole(const QString &role) { m_role = role; }
    void setPermissions(const QStringList &permissions) { m_permissions = permissions; }
    void setDepartment(const QString &dept) { m_department = dept; }
    void setOrganization(const QString &org) { m_organization = org; }
    void setStatus(const QString &status) { m_status = status; }
    void setLastLogin(const QDateTime &time) { m_lastLogin = time; }
    void setCreatedAt(const QDateTime &time) { m_createdAt = time; }
    void setUpdatedAt(const QDateTime &time) { m_updatedAt = time; }
    
    // 角色显示名称
    static QString getRoleDisplayName(const QString &role);
    static QString getStatusDisplayName(const QString &status);

private:
    int m_id;
    QString m_username;
    QString m_passwordHash;
    QString m_realName;
    QString m_email;
    QString m_phone;
    QString m_role;
    QStringList m_permissions;
    QString m_department;
    QString m_organization;
    QString m_status;
    QDateTime m_lastLogin;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

#endif // USER_H

