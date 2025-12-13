#include "permissionmanager.h"
#include "sessionmanager.h"

// 权限常量定义
const QString PermissionManager::VIEW_MAP = "view_map";
const QString PermissionManager::VIEW_PIPELINE = "view_pipeline";
const QString PermissionManager::VIEW_FACILITY = "view_facility";
const QString PermissionManager::CREATE_PIPELINE = "create_pipeline";
const QString PermissionManager::EDIT_PIPELINE = "edit_pipeline";
const QString PermissionManager::DELETE_PIPELINE = "delete_pipeline";
const QString PermissionManager::CREATE_FACILITY = "create_facility";
const QString PermissionManager::EDIT_FACILITY = "edit_facility";
const QString PermissionManager::DELETE_FACILITY = "delete_facility";
const QString PermissionManager::CREATE_WORKORDER = "create_workorder";
const QString PermissionManager::EDIT_WORKORDER = "edit_workorder";
const QString PermissionManager::DELETE_WORKORDER = "delete_workorder";
const QString PermissionManager::ASSIGN_WORKORDER = "assign_workorder";
const QString PermissionManager::START_WORKORDER = "start_workorder";
const QString PermissionManager::COMPLETE_WORKORDER = "complete_workorder";
const QString PermissionManager::VIEW_STATISTICS = "view_statistics";
const QString PermissionManager::EXPORT_DATA = "export_data";
const QString PermissionManager::MANAGE_USERS = "manage_users";
const QString PermissionManager::SYSTEM_SETTINGS = "system_settings";

// 权限检查方法
bool PermissionManager::canViewMap()
{
    return SessionManager::instance().hasPermission(VIEW_MAP);
}

bool PermissionManager::canCreatePipeline()
{
    return SessionManager::instance().hasPermission(CREATE_PIPELINE);
}

bool PermissionManager::canEditPipeline()
{
    return SessionManager::instance().hasPermission(EDIT_PIPELINE);
}

bool PermissionManager::canDeletePipeline()
{
    return SessionManager::instance().hasPermission(DELETE_PIPELINE);
}

bool PermissionManager::canCreateFacility()
{
    return SessionManager::instance().hasPermission(CREATE_FACILITY);
}

bool PermissionManager::canEditFacility()
{
    return SessionManager::instance().hasPermission(EDIT_FACILITY);
}

bool PermissionManager::canDeleteFacility()
{
    return SessionManager::instance().hasPermission(DELETE_FACILITY);
}

bool PermissionManager::canCreateWorkOrder()
{
    return SessionManager::instance().hasPermission(CREATE_WORKORDER);
}

bool PermissionManager::canEditWorkOrder()
{
    return SessionManager::instance().hasPermission(EDIT_WORKORDER);
}

bool PermissionManager::canDeleteWorkOrder()
{
    return SessionManager::instance().hasPermission(DELETE_WORKORDER);
}

bool PermissionManager::canAssignWorkOrder()
{
    return SessionManager::instance().hasPermission(ASSIGN_WORKORDER);
}

bool PermissionManager::canStartWorkOrder()
{
    return SessionManager::instance().hasPermission(START_WORKORDER);
}

bool PermissionManager::canCompleteWorkOrder()
{
    return SessionManager::instance().hasPermission(COMPLETE_WORKORDER);
}

bool PermissionManager::canViewStatistics()
{
    return SessionManager::instance().hasPermission(VIEW_STATISTICS);
}

bool PermissionManager::canExportData()
{
    return SessionManager::instance().hasPermission(EXPORT_DATA);
}

bool PermissionManager::canManageUsers()
{
    return SessionManager::instance().hasPermission(MANAGE_USERS);
}

bool PermissionManager::canAccessSettings()
{
    return SessionManager::instance().hasPermission(SYSTEM_SETTINGS);
}

