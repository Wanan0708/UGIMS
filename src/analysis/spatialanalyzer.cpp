#include "spatialanalyzer.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include <QtMath>
#include <QDebug>
#include <QLineF>

// 地球半径（米）
static const double EARTH_RADIUS = 6378137.0;

SpatialAnalyzer::SpatialAnalyzer(QObject *parent)
    : QObject(parent)
    , m_useCaching(true)
{
}

SpatialAnalyzer::~SpatialAnalyzer()
{
}

// ========================================
// 基础空间计算
// ========================================

double SpatialAnalyzer::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    // 使用 Haversine 公式计算球面距离
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLon = qDegreesToRadians(lon2 - lon1);
    
    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(qDegreesToRadians(lat1)) * qCos(qDegreesToRadians(lat2)) *
               qSin(dLon / 2) * qSin(dLon / 2);
    
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    
    return EARTH_RADIUS * c;
}

double SpatialAnalyzer::pointToLineDistance(const QPointF &point, const QLineF &line)
{
    // 计算点到线段的垂直距离
    QPointF p1 = line.p1();
    QPointF p2 = line.p2();
    
    double lineLengthSquared = QLineF(p1, p2).length() * QLineF(p1, p2).length();
    
    if (lineLengthSquared == 0.0) {
        return QLineF(point, p1).length();
    }
    
    // 计算投影点
    double t = QPointF::dotProduct(point - p1, p2 - p1) / lineLengthSquared;
    t = qMax(0.0, qMin(1.0, t));
    
    QPointF projection = p1 + t * (p2 - p1);
    return QLineF(point, projection).length();
}

bool SpatialAnalyzer::isPointInPolygon(const QPointF &point, const QPolygonF &polygon)
{
    if (polygon.isEmpty()) {
        return false;
    }
    
    return polygon.containsPoint(point, Qt::OddEvenFill);
}

QPolygonF SpatialAnalyzer::createCircleBuffer(const QPointF &center, double radius, int segments)
{
    QPolygonF circle;
    
    for (int i = 0; i < segments; ++i) {
        double angle = 2.0 * M_PI * i / segments;
        double x = center.x() + radius * qCos(angle);
        double y = center.y() + radius * qSin(angle);
        circle << QPointF(x, y);
    }
    
    // 闭合多边形
    if (!circle.isEmpty()) {
        circle << circle.first();
    }
    
    return circle;
}

QPolygonF SpatialAnalyzer::createLineBuffer(const QList<QPointF> &line, double width)
{
    if (line.size() < 2) {
        return QPolygonF();
    }
    
    QPolygonF buffer;
    double halfWidth = width / 2.0;
    
    // 简化实现：为每个线段创建矩形缓冲区
    for (int i = 0; i < line.size() - 1; ++i) {
        QPointF p1 = line[i];
        QPointF p2 = line[i + 1];
        
        // 计算垂直向量
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double length = qSqrt(dx * dx + dy * dy);
        
        if (length > 0) {
            double nx = -dy / length * halfWidth;
            double ny = dx / length * halfWidth;
            
            buffer << QPointF(p1.x() + nx, p1.y() + ny);
            buffer << QPointF(p2.x() + nx, p2.y() + ny);
        }
    }
    
    // 反向添加另一侧
    for (int i = line.size() - 2; i >= 0; --i) {
        QPointF p1 = line[i];
        QPointF p2 = line[i + 1];
        
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double length = qSqrt(dx * dx + dy * dy);
        
        if (length > 0) {
            double nx = -dy / length * halfWidth;
            double ny = dx / length * halfWidth;
            
            buffer << QPointF(p2.x() - nx, p2.y() - ny);
            buffer << QPointF(p1.x() - nx, p1.y() - ny);
        }
    }
    
    return buffer;
}

double SpatialAnalyzer::calculatePolygonArea(const QPolygonF &polygon)
{
    if (polygon.size() < 3) {
        return 0.0;
    }
    
    double area = 0.0;
    int n = polygon.size();
    
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += polygon[i].x() * polygon[j].y();
        area -= polygon[j].x() * polygon[i].y();
    }
    
    return qAbs(area) / 2.0;
}

double SpatialAnalyzer::calculateLineLength(const QList<QPointF> &line)
{
    if (line.size() < 2) {
        return 0.0;
    }
    
    double totalLength = 0.0;
    
    for (int i = 0; i < line.size() - 1; ++i) {
        totalLength += QLineF(line[i], line[i + 1]).length();
    }
    
    return totalLength;
}

// ========================================
// 坐标转换
// ========================================

QPointF SpatialAnalyzer::latLonToMercator(double lat, double lon)
{
    double x = lon * EARTH_RADIUS * M_PI / 180.0;
    double y = qLn(qTan((90.0 + lat) * M_PI / 360.0)) * EARTH_RADIUS;
    return QPointF(x, y);
}

void SpatialAnalyzer::mercatorToLatLon(const QPointF &mercator, double &lat, double &lon)
{
    lon = mercator.x() / EARTH_RADIUS * 180.0 / M_PI;
    lat = 360.0 / M_PI * qAtan(qExp(mercator.y() / EARTH_RADIUS)) - 90.0;
}

// ========================================
// 空间查询
// ========================================

QList<QString> SpatialAnalyzer::performQuery(const QueryParams &params)
{
    QList<QString> results;
    
    switch (params.type) {
    case QueryType::PointQuery:
        results = queryByPoint(params.center, 10.0); // 默认10米容差
        break;
        
    case QueryType::BoxQuery:
        results = queryByBox(params.boundingBox);
        break;
        
    case QueryType::CircleQuery:
        results = queryByCircle(params.center, params.radius);
        break;
        
    case QueryType::BufferQuery:
        results = queryByBuffer(params.center, params.bufferDistance);
        break;
        
    default:
        qWarning() << "Unknown query type";
        break;
    }
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByPoint(const QPointF &point, double tolerance)
{
    QList<QString> results;
    
    qDebug() << "[SpatialAnalyzer] Query by point:" << point << "tolerance:" << tolerance << "meters";
    
    try {
        // 将容差从米转换为度（近似，适用于小范围）
        // 1度经度 ≈ 111km，1度纬度 ≈ 111km
        double toleranceDegrees = tolerance / 111000.0;
        
        // 查询管线
        PipelineDAO pipelineDao;
        QVector<Pipeline> pipelines = pipelineDao.findNearPoint(point.x(), point.y(), tolerance, 100);
        for (const Pipeline &pipeline : pipelines) {
            // 计算管线到点的实际距离
            double minDistance = std::numeric_limits<double>::max();
            QVector<QPointF> coords = pipeline.coordinates();
            
            for (int i = 0; i < coords.size() - 1; ++i) {
                QLineF line(coords[i], coords[i + 1]);
                double dist = pointToLineDistance(point, line);
                // 转换为米（近似）
                double distMeters = dist * 111000.0;
                if (distMeters < minDistance) {
                    minDistance = distMeters;
                }
            }
            
            // 只添加在容差范围内的管线
            if (minDistance <= tolerance) {
                results.append(pipeline.pipelineId());
            }
        }
        
        // 查询设施
        FacilityDAO facilityDao;
        QVector<Facility> facilities = facilityDao.findNearPoint(point.x(), point.y(), tolerance, 100);
        for (const Facility &facility : facilities) {
            QPointF facilityPos = facility.coordinate();
            double dist = calculateDistance(point.y(), point.x(), facilityPos.y(), facilityPos.x());
            
            // 只添加在容差范围内的设施
            if (dist <= tolerance) {
                results.append(facility.facilityId());
            }
        }
        
        qDebug() << "[SpatialAnalyzer] Found" << results.size() << "entities near point";
        
    } catch (const std::exception &e) {
        qWarning() << "[SpatialAnalyzer] Query by point error:" << e.what();
        emit analysisError(QString("点查询失败: %1").arg(e.what()));
    }
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByBox(const QRectF &box)
{
    QList<QString> results;
    
    qDebug() << "[SpatialAnalyzer] Query by box:" << box;
    
    try {
        // 查询管线
        PipelineDAO pipelineDao;
        QVector<Pipeline> pipelines = pipelineDao.findByBounds(box, 1000);
        for (const Pipeline &pipeline : pipelines) {
            results.append(pipeline.pipelineId());
        }
        
        // 查询设施
        FacilityDAO facilityDao;
        QVector<Facility> facilities = facilityDao.findByBounds(box, 1000);
        for (const Facility &facility : facilities) {
            results.append(facility.facilityId());
        }
        
        qDebug() << "[SpatialAnalyzer] Found" << results.size() << "entities in box";
        
    } catch (const std::exception &e) {
        qWarning() << "[SpatialAnalyzer] Query by box error:" << e.what();
        emit analysisError(QString("矩形查询失败: %1").arg(e.what()));
    }
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByCircle(const QPointF &center, double radius)
{
    QList<QString> results;
    
    qDebug() << "[SpatialAnalyzer] Query by circle: center=" << center << "radius=" << radius << "meters";
    
    try {
        // 查询管线
        PipelineDAO pipelineDao;
        QVector<Pipeline> pipelines = pipelineDao.findNearPoint(center.x(), center.y(), radius, 1000);
        for (const Pipeline &pipeline : pipelines) {
            // 检查管线是否与圆形区域相交
            bool intersects = false;
            QVector<QPointF> coords = pipeline.coordinates();
            
            for (const QPointF &coord : coords) {
                double dist = calculateDistance(center.y(), center.x(), coord.y(), coord.x());
                if (dist <= radius) {
                    intersects = true;
                    break;
                }
            }
            
            if (intersects) {
                results.append(pipeline.pipelineId());
            }
        }
        
        // 查询设施
        FacilityDAO facilityDao;
        QVector<Facility> facilities = facilityDao.findNearPoint(center.x(), center.y(), radius, 1000);
        for (const Facility &facility : facilities) {
            QPointF facilityPos = facility.coordinate();
            double dist = calculateDistance(center.y(), center.x(), facilityPos.y(), facilityPos.x());
            
            if (dist <= radius) {
                results.append(facility.facilityId());
            }
        }
        
        qDebug() << "[SpatialAnalyzer] Found" << results.size() << "entities in circle";
        
    } catch (const std::exception &e) {
        qWarning() << "[SpatialAnalyzer] Query by circle error:" << e.what();
        emit analysisError(QString("圆形查询失败: %1").arg(e.what()));
    }
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByBuffer(const QPointF &point, double bufferDistance)
{
    QList<QString> results;
    
    qDebug() << "[SpatialAnalyzer] Query by buffer: point=" << point << "distance=" << bufferDistance << "meters";
    
    try {
        // 缓冲区查询实际上就是圆形查询
        // 查询管线
        PipelineDAO pipelineDao;
        QVector<Pipeline> pipelines = pipelineDao.findNearPoint(point.x(), point.y(), bufferDistance, 1000);
        for (const Pipeline &pipeline : pipelines) {
            // 计算管线到点的最小距离
            double minDistance = std::numeric_limits<double>::max();
            QVector<QPointF> coords = pipeline.coordinates();
            
            for (int i = 0; i < coords.size() - 1; ++i) {
                QLineF line(coords[i], coords[i + 1]);
                double dist = pointToLineDistance(point, line);
                // 转换为米（近似）
                double distMeters = dist * 111000.0;
                if (distMeters < minDistance) {
                    minDistance = distMeters;
                }
            }
            
            // 只添加在缓冲区范围内的管线
            if (minDistance <= bufferDistance) {
                results.append(pipeline.pipelineId());
            }
        }
        
        // 查询设施
        FacilityDAO facilityDao;
        QVector<Facility> facilities = facilityDao.findNearPoint(point.x(), point.y(), bufferDistance, 1000);
        for (const Facility &facility : facilities) {
            QPointF facilityPos = facility.coordinate();
            double dist = calculateDistance(point.y(), point.x(), facilityPos.y(), facilityPos.x());
            
            if (dist <= bufferDistance) {
                results.append(facility.facilityId());
            }
        }
        
        qDebug() << "[SpatialAnalyzer] Found" << results.size() << "entities in buffer";
        
    } catch (const std::exception &e) {
        qWarning() << "[SpatialAnalyzer] Query by buffer error:" << e.what();
        emit analysisError(QString("缓冲区查询失败: %1").arg(e.what()));
    }
    
    return results;
}

// ========================================
// 辅助方法
// ========================================

void SpatialAnalyzer::emitProgress(int current, int total, const QString &message)
{
    emit analysisProgress(current, total, message);
}

