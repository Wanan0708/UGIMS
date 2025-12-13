#ifndef WORKORDERDAO_H
#define WORKORDERDAO_H

#include "dao/basedao.h"
#include "core/models/workorder.h"
#include <QVector>
#include <QDateTime>

/**
 * @brief 工单数据访问对象
 * 提供工单相关的数据库操作
 */
class WorkOrderDAO : public BaseDAO<WorkOrder>
{
public:
    WorkOrderDAO();

    // 实现基类纯虚函数
    WorkOrder fromQuery(QSqlQuery &query) override;
    QVariantMap toVariantMap(const WorkOrder &workOrder) override;

    // 根据工单ID查找
    WorkOrder findByOrderId(const QString &orderId);
    
    // 根据状态查找工单
    QVector<WorkOrder> findByStatus(const QString &status, int limit = 1000);
    
    // 根据类型查找工单
    QVector<WorkOrder> findByType(const QString &type, int limit = 1000);
    
    // 根据优先级查找工单
    QVector<WorkOrder> findByPriority(const QString &priority, int limit = 1000);
    
    // 根据分配人查找工单
    QVector<WorkOrder> findByAssignedTo(const QString &assignedTo, int limit = 1000);
    
    // 查找指定日期范围内的工单
    QVector<WorkOrder> findByDateRange(const QDateTime &startDate, const QDateTime &endDate, int limit = 1000);
    
    // 查找所有工单（支持分页）
    QVector<WorkOrder> findAll(int limit = 1000, int offset = 0);
    
    // 统计各状态工单数量
    QMap<QString, int> countByStatus();
    
    // 统计各类型工单数量
    QMap<QString, int> countByType();
    
    // 生成新的工单ID
    QString generateOrderId();
    
    // 重写insert和update方法以处理PostGIS字段
    bool insert(const WorkOrder &workOrder);
    bool update(const WorkOrder &workOrder);
    bool deleteByOrderId(const QString &orderId);

private:
    // 确保表存在，如果不存在则创建
    void ensureTableExists();
    
    // 创建工单表
    void createTable();
    
    // 解析PostGIS POINT为QPointF
    QPointF parsePointWkt(const QString &wkt);
    
    // 将QPointF转换为PostGIS POINT WKT
    QString toPointWkt(const QPointF &point);
    
    // 解析PostgreSQL数组为QStringList
    QStringList parseArray(const QString &arrayStr);
    
    // 将QStringList转换为PostgreSQL数组字符串
    QString toArrayString(const QStringList &list);
};

#endif // WORKORDERDAO_H

