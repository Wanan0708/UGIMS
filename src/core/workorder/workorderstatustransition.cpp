#include "workorderstatustransition.h"
#include "core/common/logger.h"
#include <QDebug>
#include <QDateTime>

WorkOrderStatusTransition::WorkOrderStatusTransition(QObject *parent)
    : QObject(parent)
{
    initializeTransitionRules();
    initializeActionNames();
}

WorkOrderStatusTransition::~WorkOrderStatusTransition()
{
}

void WorkOrderStatusTransition::initializeTransitionRules()
{
    // 定义状态转换规则
    // 待分配 -> 已分配、已取消
    m_transitionRules[WorkOrder::STATUS_PENDING] = {
        WorkOrder::STATUS_ASSIGNED,
        WorkOrder::STATUS_CANCELLED
    };
    
    // 已分配 -> 进行中、已取消
    m_transitionRules[WorkOrder::STATUS_ASSIGNED] = {
        WorkOrder::STATUS_IN_PROGRESS,
        WorkOrder::STATUS_CANCELLED
    };
    
    // 进行中 -> 已完成、已取消
    m_transitionRules[WorkOrder::STATUS_IN_PROGRESS] = {
        WorkOrder::STATUS_COMPLETED,
        WorkOrder::STATUS_CANCELLED
    };
    
    // 已完成 -> 无（终态）
    m_transitionRules[WorkOrder::STATUS_COMPLETED] = {};
    
    // 已取消 -> 无（终态）
    m_transitionRules[WorkOrder::STATUS_CANCELLED] = {};
}

void WorkOrderStatusTransition::initializeActionNames()
{
    // 定义状态转换操作的显示名称
    m_actionNames[qMakePair(WorkOrder::STATUS_PENDING, WorkOrder::STATUS_ASSIGNED)] = "派发";
    m_actionNames[qMakePair(WorkOrder::STATUS_PENDING, WorkOrder::STATUS_CANCELLED)] = "取消";
    
    m_actionNames[qMakePair(WorkOrder::STATUS_ASSIGNED, WorkOrder::STATUS_IN_PROGRESS)] = "开始";
    m_actionNames[qMakePair(WorkOrder::STATUS_ASSIGNED, WorkOrder::STATUS_CANCELLED)] = "取消";
    
    m_actionNames[qMakePair(WorkOrder::STATUS_IN_PROGRESS, WorkOrder::STATUS_COMPLETED)] = "完成";
    m_actionNames[qMakePair(WorkOrder::STATUS_IN_PROGRESS, WorkOrder::STATUS_CANCELLED)] = "取消";
}

bool WorkOrderStatusTransition::canTransition(const QString &fromStatus, const QString &toStatus) const
{
    if (fromStatus == toStatus) {
        return false;  // 不能转换到相同状态
    }
    
    QStringList availableTransitions = m_transitionRules.value(fromStatus);
    return availableTransitions.contains(toStatus);
}

QStringList WorkOrderStatusTransition::getAvailableTransitions(const QString &fromStatus) const
{
    return m_transitionRules.value(fromStatus);
}

QString WorkOrderStatusTransition::getTransitionActionName(const QString &fromStatus, const QString &toStatus) const
{
    QPair<QString, QString> key = qMakePair(fromStatus, toStatus);
    QString actionName = m_actionNames.value(key);
    
    if (actionName.isEmpty()) {
        // 如果没有定义操作名称，返回默认名称
        if (toStatus == WorkOrder::STATUS_ASSIGNED) return "派发";
        if (toStatus == WorkOrder::STATUS_IN_PROGRESS) return "开始";
        if (toStatus == WorkOrder::STATUS_COMPLETED) return "完成";
        if (toStatus == WorkOrder::STATUS_CANCELLED) return "取消";
        return "转换";
    }
    
    return actionName;
}

QString WorkOrderStatusTransition::validateTransition(const WorkOrder &workOrder, const QString &toStatus) const
{
    QString currentStatus = workOrder.status();
    
    // 检查是否可以转换
    if (!canTransition(currentStatus, toStatus)) {
        QString fromName = (currentStatus == WorkOrder::STATUS_PENDING) ? "待分配" :
                          (currentStatus == WorkOrder::STATUS_ASSIGNED) ? "已分配" :
                          (currentStatus == WorkOrder::STATUS_IN_PROGRESS) ? "进行中" :
                          (currentStatus == WorkOrder::STATUS_COMPLETED) ? "已完成" :
                          (currentStatus == WorkOrder::STATUS_CANCELLED) ? "已取消" : currentStatus;
        QString toName = (toStatus == WorkOrder::STATUS_PENDING) ? "待分配" :
                        (toStatus == WorkOrder::STATUS_ASSIGNED) ? "已分配" :
                        (toStatus == WorkOrder::STATUS_IN_PROGRESS) ? "进行中" :
                        (toStatus == WorkOrder::STATUS_COMPLETED) ? "已完成" :
                        (toStatus == WorkOrder::STATUS_CANCELLED) ? "已取消" : toStatus;
        return QString("工单状态不能从 '%1' 转换到 '%2'").arg(fromName).arg(toName);
    }
    
    // 检查前置条件
    if (toStatus == WorkOrder::STATUS_ASSIGNED) {
        // 派发需要指定分配人
        if (workOrder.assignedTo().isEmpty()) {
            return "派发工单需要指定分配人";
        }
    }
    
    if (toStatus == WorkOrder::STATUS_IN_PROGRESS) {
        // 开始执行需要已分配
        if (workOrder.assignedTo().isEmpty()) {
            return "开始执行需要工单已分配";
        }
    }
    
    if (toStatus == WorkOrder::STATUS_COMPLETED) {
        // 完成需要填写工作结果
        if (workOrder.workResult().isEmpty()) {
            return "完成工单需要填写工作结果";
        }
    }
    
    return QString();  // 验证通过
}

bool WorkOrderStatusTransition::performTransition(WorkOrder &workOrder, const QString &toStatus, const QString &operatorName)
{
    QString fromStatus = workOrder.status();
    
    // 验证转换
    QString error = validateTransition(workOrder, toStatus);
    if (!error.isEmpty()) {
        qWarning() << "[WorkOrderStatusTransition] Validation failed:" << error;
        return false;
    }
    
    // 执行状态转换
    workOrder.setStatus(toStatus);
    workOrder.setUpdatedAt(QDateTime::currentDateTime());
    
    // 根据目标状态更新相关字段
    if (toStatus == WorkOrder::STATUS_ASSIGNED) {
        // 派发时记录分配信息
        if (workOrder.assignedAt().isNull()) {
            workOrder.setAssignedAt(QDateTime::currentDateTime());
        }
        if (workOrder.assignedBy().isEmpty()) {
            workOrder.setAssignedBy(operatorName);
        }
    } else if (toStatus == WorkOrder::STATUS_IN_PROGRESS) {
        // 开始执行时记录实际开始时间
        if (workOrder.actualStartTime().isNull()) {
            workOrder.setActualStartTime(QDateTime::currentDateTime());
        }
    } else if (toStatus == WorkOrder::STATUS_COMPLETED) {
        // 完成时记录实际结束时间
        if (workOrder.actualEndTime().isNull()) {
            workOrder.setActualEndTime(QDateTime::currentDateTime());
        }
        // 如果完成度未设置，设置为100%
        if (workOrder.completionRate() < 100) {
            workOrder.setCompletionRate(100);
        }
    }
    
    // 发送状态变更信号
    emit statusChanged(workOrder.orderId(), fromStatus, toStatus, operatorName);
    
    LOG_INFO(QString("Work order %1 status changed: %2 -> %3 by %4")
             .arg(workOrder.orderId())
             .arg(fromStatus)
             .arg(toStatus)
             .arg(operatorName));
    
    return true;
}

