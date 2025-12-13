#ifndef WORKORDERSTATUSTRANSITION_H
#define WORKORDERSTATUSTRANSITION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include "core/models/workorder.h"

/**
 * @brief 工单状态转换管理器
 * 负责工单状态流转的验证和转换逻辑
 */
class WorkOrderStatusTransition : public QObject
{
    Q_OBJECT

public:
    explicit WorkOrderStatusTransition(QObject *parent = nullptr);
    ~WorkOrderStatusTransition();

    /**
     * @brief 检查状态转换是否有效
     * @param fromStatus 当前状态
     * @param toStatus 目标状态
     * @return 是否可以转换
     */
    bool canTransition(const QString &fromStatus, const QString &toStatus) const;

    /**
     * @brief 获取从指定状态可以转换到的所有状态
     * @param fromStatus 当前状态
     * @return 可转换的状态列表
     */
    QStringList getAvailableTransitions(const QString &fromStatus) const;

    /**
     * @brief 获取状态转换的显示名称
     * @param fromStatus 当前状态
     * @param toStatus 目标状态
     * @return 转换操作的显示名称（如"派发"、"开始"、"完成"等）
     */
    QString getTransitionActionName(const QString &fromStatus, const QString &toStatus) const;

    /**
     * @brief 验证状态转换的前置条件
     * @param workOrder 工单对象
     * @param toStatus 目标状态
     * @return 错误信息，如果为空则表示验证通过
     */
    QString validateTransition(const WorkOrder &workOrder, const QString &toStatus) const;

    /**
     * @brief 执行状态转换（更新工单对象）
     * @param workOrder 工单对象（会被修改）
     * @param toStatus 目标状态
     * @param operatorName 操作人姓名
     * @return 是否成功
     */
    bool performTransition(WorkOrder &workOrder, const QString &toStatus, const QString &operatorName);

signals:
    /**
     * @brief 状态转换完成信号
     * @param orderId 工单编号
     * @param fromStatus 原状态
     * @param toStatus 新状态
     * @param operatorName 操作人
     */
    void statusChanged(const QString &orderId, const QString &fromStatus, 
                      const QString &toStatus, const QString &operatorName);

private:
    void initializeTransitionRules();
    void initializeActionNames();

private:
    // 状态转换规则：fromStatus -> [toStatus1, toStatus2, ...]
    QMap<QString, QStringList> m_transitionRules;
    
    // 状态转换操作名称：(fromStatus, toStatus) -> actionName
    QMap<QPair<QString, QString>, QString> m_actionNames;
};

#endif // WORKORDERSTATUSTRANSITION_H

