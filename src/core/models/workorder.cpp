#include "workorder.h"

// 工单类型常量
const QString WorkOrder::TYPE_INSPECTION = "inspection";
const QString WorkOrder::TYPE_MAINTENANCE = "maintenance";
const QString WorkOrder::TYPE_EMERGENCY = "emergency";
const QString WorkOrder::TYPE_RENOVATION = "renovation";

// 优先级常量
const QString WorkOrder::PRIORITY_LOW = "low";
const QString WorkOrder::PRIORITY_NORMAL = "normal";
const QString WorkOrder::PRIORITY_HIGH = "high";
const QString WorkOrder::PRIORITY_URGENT = "urgent";

// 状态常量
const QString WorkOrder::STATUS_PENDING = "pending";
const QString WorkOrder::STATUS_ASSIGNED = "assigned";
const QString WorkOrder::STATUS_IN_PROGRESS = "in_progress";
const QString WorkOrder::STATUS_COMPLETED = "completed";
const QString WorkOrder::STATUS_CANCELLED = "cancelled";

WorkOrder::WorkOrder()
    : m_id(0)
    , m_completionRate(0)
    , m_location(QPointF())
{
}

