#ifndef PERMISSIONMANAGER_H
#define PERMISSIONMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief 权限管理器
 * 定义和管理系统权限常量
 */
class PermissionManager : public QObject
{
    Q_OBJECT

public:
    // 权限常量定义
    static const QString VIEW_MAP;
    static const QString VIEW_PIPELINE;
    static const QString VIEW_FACILITY;
    static const QString CREATE_PIPELINE;
    static const QString EDIT_PIPELINE;
    static const QString DELETE_PIPELINE;
    static const QString CREATE_FACILITY;
    static const QString EDIT_FACILITY;
    static const QString DELETE_FACILITY;
    static const QString CREATE_WORKORDER;
    static const QString EDIT_WORKORDER;
    static const QString DELETE_WORKORDER;
    static const QString ASSIGN_WORKORDER;
    static const QString START_WORKORDER;
    static const QString COMPLETE_WORKORDER;
    static const QString VIEW_STATISTICS;
    static const QString EXPORT_DATA;
    static const QString MANAGE_USERS;
    static const QString SYSTEM_SETTINGS;
    
    // 检查权限的便捷方法
    static bool canViewMap();
    static bool canCreatePipeline();
    static bool canEditPipeline();
    static bool canDeletePipeline();
    static bool canCreateFacility();
    static bool canEditFacility();
    static bool canDeleteFacility();
    static bool canCreateWorkOrder();
    static bool canEditWorkOrder();
    static bool canDeleteWorkOrder();
    static bool canAssignWorkOrder();
    static bool canStartWorkOrder();
    static bool canCompleteWorkOrder();
    static bool canViewStatistics();
    static bool canExportData();
    static bool canManageUsers();
    static bool canAccessSettings();
};

#endif // PERMISSIONMANAGER_H

