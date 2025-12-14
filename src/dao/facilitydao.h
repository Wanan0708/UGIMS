#ifndef FACILITYDAO_H
#define FACILITYDAO_H

#include "dao/basedao.h"
#include "core/models/facility.h"
#include <QRectF>

/**
 * @brief 设施数据访问对象
 * 提供设施相关的数据库操作
 */
class FacilityDAO : public BaseDAO<Facility>
{
public:
    FacilityDAO();

    // 实现基类纯虚函数
    Facility fromQuery(QSqlQuery &query) override;
    QVariantMap toVariantMap(const Facility &facility) override;

    // 查找所有设施
    QVector<Facility> findAll(int limit = 1000);
    
    // 根据类型查找设施
    QVector<Facility> findByType(const QString &type, int limit = 1000);

    // 根据边界框查找设施（空间查询）
    QVector<Facility> findByBounds(const QRectF &bounds, int limit = 1000);

    // 根据设施ID查找
    Facility findByFacilityId(const QString &facilityId);

    // 根据关联管线ID查找设施
    QVector<Facility> findByPipelineId(const QString &pipelineId, int limit = 1000);

    // 根据状态查找设施
    QVector<Facility> findByStatus(const QString &status, int limit = 1000);

    // 查找健康度低于阈值的设施
    QVector<Facility> findByHealthScore(int maxScore, int limit = 1000);

    // 查找指定点周围的设施（缓冲区查询）
    QVector<Facility> findNearPoint(double lon, double lat, double radiusMeters, int limit = 100);

    // 统计各类型设施数量
    QMap<QString, int> countByType();
    
    // 重写insert方法以处理PostGIS字段
    bool insert(const Facility &facility);
    
    // 重写update方法以处理PostGIS字段
    bool update(const Facility &facility, int id);

private:
    // 解析WKT格式的POINT为坐标点
    QPointF parsePointWkt(const QString &wkt);

    // 将坐标点转换为WKT格式
    QString toPointWkt(const QPointF &coordinate);
};

#endif // FACILITYDAO_H

