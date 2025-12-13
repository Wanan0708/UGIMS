#ifndef WORKORDER_H
#define WORKORDER_H

#include <QString>
#include <QDateTime>
#include <QPointF>
#include <QStringList>

/**
 * @brief 工单数据模型
 * 对应数据库 work_orders 表
 */
class WorkOrder
{
public:
    WorkOrder();

    // Getter
    int id() const { return m_id; }
    QString orderId() const { return m_orderId; }
    QString orderTitle() const { return m_orderTitle; }
    QString orderType() const { return m_orderType; }
    QString priority() const { return m_priority; }
    
    // 关联资产
    QString assetType() const { return m_assetType; }
    QString assetId() const { return m_assetId; }
    QPointF location() const { return m_location; }
    
    // 工单内容
    QString description() const { return m_description; }
    QString requiredActions() const { return m_requiredActions; }
    
    // 人员分配
    QString assignedTo() const { return m_assignedTo; }
    QDateTime assignedAt() const { return m_assignedAt; }
    QString assignedBy() const { return m_assignedBy; }
    
    // 状态流转
    QString status() const { return m_status; }
    
    // 时间节点
    QDateTime planStartTime() const { return m_planStartTime; }
    QDateTime planEndTime() const { return m_planEndTime; }
    QDateTime actualStartTime() const { return m_actualStartTime; }
    QDateTime actualEndTime() const { return m_actualEndTime; }
    
    // 完成情况
    int completionRate() const { return m_completionRate; }
    QString workResult() const { return m_workResult; }
    QStringList photos() const { return m_photos; }
    
    // 审核
    QString reviewedBy() const { return m_reviewedBy; }
    QDateTime reviewedAt() const { return m_reviewedAt; }
    QString reviewResult() const { return m_reviewResult; }
    QString reviewComments() const { return m_reviewComments; }
    
    // 元数据
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }
    QString createdBy() const { return m_createdBy; }

    // Setter
    void setId(int id) { m_id = id; }
    void setOrderId(const QString &orderId) { m_orderId = orderId; }
    void setOrderTitle(const QString &title) { m_orderTitle = title; }
    void setOrderType(const QString &type) { m_orderType = type; }
    void setPriority(const QString &priority) { m_priority = priority; }
    
    void setAssetType(const QString &type) { m_assetType = type; }
    void setAssetId(const QString &id) { m_assetId = id; }
    void setLocation(const QPointF &loc) { m_location = loc; }
    
    void setDescription(const QString &desc) { m_description = desc; }
    void setRequiredActions(const QString &actions) { m_requiredActions = actions; }
    
    void setAssignedTo(const QString &to) { m_assignedTo = to; }
    void setAssignedAt(const QDateTime &at) { m_assignedAt = at; }
    void setAssignedBy(const QString &by) { m_assignedBy = by; }
    
    void setStatus(const QString &status) { m_status = status; }
    
    void setPlanStartTime(const QDateTime &time) { m_planStartTime = time; }
    void setPlanEndTime(const QDateTime &time) { m_planEndTime = time; }
    void setActualStartTime(const QDateTime &time) { m_actualStartTime = time; }
    void setActualEndTime(const QDateTime &time) { m_actualEndTime = time; }
    
    void setCompletionRate(int rate) { m_completionRate = rate; }
    void setWorkResult(const QString &result) { m_workResult = result; }
    void setPhotos(const QStringList &photos) { m_photos = photos; }
    
    void setReviewedBy(const QString &by) { m_reviewedBy = by; }
    void setReviewedAt(const QDateTime &at) { m_reviewedAt = at; }
    void setReviewResult(const QString &result) { m_reviewResult = result; }
    void setReviewComments(const QString &comments) { m_reviewComments = comments; }
    
    void setCreatedAt(const QDateTime &at) { m_createdAt = at; }
    void setUpdatedAt(const QDateTime &at) { m_updatedAt = at; }
    void setCreatedBy(const QString &by) { m_createdBy = by; }

    // 工具方法
    bool isValid() const { return !m_orderId.isEmpty(); }
    
    // 工单类型常量
    static const QString TYPE_INSPECTION;
    static const QString TYPE_MAINTENANCE;
    static const QString TYPE_EMERGENCY;
    static const QString TYPE_RENOVATION;
    
    // 优先级常量
    static const QString PRIORITY_LOW;
    static const QString PRIORITY_NORMAL;
    static const QString PRIORITY_HIGH;
    static const QString PRIORITY_URGENT;
    
    // 状态常量
    static const QString STATUS_PENDING;
    static const QString STATUS_ASSIGNED;
    static const QString STATUS_IN_PROGRESS;
    static const QString STATUS_COMPLETED;
    static const QString STATUS_CANCELLED;

private:
    int m_id;
    QString m_orderId;
    QString m_orderTitle;
    QString m_orderType;
    QString m_priority;
    
    QString m_assetType;
    QString m_assetId;
    QPointF m_location;
    
    QString m_description;
    QString m_requiredActions;
    
    QString m_assignedTo;
    QDateTime m_assignedAt;
    QString m_assignedBy;
    
    QString m_status;
    
    QDateTime m_planStartTime;
    QDateTime m_planEndTime;
    QDateTime m_actualStartTime;
    QDateTime m_actualEndTime;
    
    int m_completionRate;
    QString m_workResult;
    QStringList m_photos;
    
    QString m_reviewedBy;
    QDateTime m_reviewedAt;
    QString m_reviewResult;
    QString m_reviewComments;
    
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString m_createdBy;
};

#endif // WORKORDER_H
