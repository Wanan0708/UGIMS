#include "user.h"
#include <QDebug>

// 静态常量定义
const QString User::ROLE_ADMIN = "admin";
const QString User::ROLE_MANAGER = "manager";
const QString User::ROLE_INSPECTOR = "inspector";
const QString User::ROLE_VIEWER = "viewer";

const QString User::STATUS_ACTIVE = "active";
const QString User::STATUS_INACTIVE = "inactive";
const QString User::STATUS_LOCKED = "locked";

User::User()
    : m_id(0)
    , m_role(ROLE_VIEWER)
    , m_status(STATUS_ACTIVE)
{
}

bool User::hasPermission(const QString &permission) const
{
    // 管理员拥有所有权限
    if (isAdmin()) {
        return true;
    }
    
    // 检查显式权限列表
    if (m_permissions.contains(permission)) {
        return true;
    }
    
    // 根据角色返回默认权限
    if (permission == "view_map" || permission == "view_pipeline" || permission == "view_facility") {
        return true;  // 所有角色都可以查看
    }
    
    if (permission == "create_workorder") {
        return isManager() || isInspector();
    }
    
    if (permission == "edit_workorder" || permission == "assign_workorder") {
        return isManager() || isAdmin();
    }
    
    if (permission == "start_workorder" || permission == "complete_workorder") {
        return isManager() || isInspector() || isAdmin();
    }
    
    if (permission == "edit_pipeline" || permission == "edit_facility") {
        return isManager() || isAdmin();
    }
    
    if (permission == "create_pipeline" || permission == "create_facility") {
        return isManager() || isAdmin();
    }
    
    if (permission == "delete_pipeline" || permission == "delete_facility" || 
        permission == "delete_workorder") {
        return isAdmin();
    }
    
    if (permission == "view_statistics" || permission == "export_data") {
        return isManager() || isAdmin();
    }
    
    if (permission == "manage_users" || permission == "system_settings") {
        return isAdmin();
    }
    
    return false;
}

QString User::getRoleDisplayName(const QString &role)
{
    if (role == ROLE_ADMIN) return "管理员";
    if (role == ROLE_MANAGER) return "管理者";
    if (role == ROLE_INSPECTOR) return "巡检员";
    if (role == ROLE_VIEWER) return "查看者";
    return role;
}

QString User::getStatusDisplayName(const QString &status)
{
    if (status == STATUS_ACTIVE) return "激活";
    if (status == STATUS_INACTIVE) return "未激活";
    if (status == STATUS_LOCKED) return "锁定";
    return status;
}

