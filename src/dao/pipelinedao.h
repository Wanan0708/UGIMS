#ifndef PIPELINEDAO_H
#define PIPELINEDAO_H

#include "dao/basedao.h"
#include "core/models/pipeline.h"
#include <QRectF>

/**
 * @brief 管线数据访问对象
 * 提供管线相关的数据库操作
 */
class PipelineDAO : public BaseDAO<Pipeline>
{
public:
    PipelineDAO();

    // 实现基类纯虚函数
    Pipeline fromQuery(QSqlQuery &query) override;
    QVariantMap toVariantMap(const Pipeline &pipeline) override;

    // 根据类型查找管线
    QVector<Pipeline> findByType(const QString &type, int limit = 1000);

    // 根据边界框查找管线（空间查询）
    QVector<Pipeline> findByBounds(const QRectF &bounds, int limit = 1000);

    // 根据管线ID查找
    Pipeline findByPipelineId(const QString &pipelineId);

    // 根据状态查找管线
    QVector<Pipeline> findByStatus(const QString &status, int limit = 1000);

    // 查找健康度低于阈值的管线
    QVector<Pipeline> findByHealthScore(int maxScore, int limit = 1000);

    // 查找指定点周围的管线（缓冲区查询）
    QVector<Pipeline> findNearPoint(double lon, double lat, double radiusMeters, int limit = 100);

    // 统计各类型管线数量
    QMap<QString, int> countByType();
    
    // 重写insert和update方法以处理PostGIS字段
    bool insert(const Pipeline &pipeline);
    bool update(const Pipeline &pipeline, int id);

private:
    // 解析WKT格式的LINESTRING为坐标点
    QVector<QPointF> parseLineStringWkt(const QString &wkt);

    // 将坐标点转换为WKT格式
    QString toLineStringWkt(const QVector<QPointF> &coordinates);
};

#endif // PIPELINEDAO_H

