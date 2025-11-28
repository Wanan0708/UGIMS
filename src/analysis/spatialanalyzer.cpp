#include "spatialanalyzer.h"
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
    
    // TODO: 实际实现需要查询数据源
    // 这里返回模拟数据
    qDebug() << "Query by point:" << point << "tolerance:" << tolerance;
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByBox(const QRectF &box)
{
    QList<QString> results;
    
    // TODO: 实际实现需要查询数据源
    qDebug() << "Query by box:" << box;
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByCircle(const QPointF &center, double radius)
{
    QList<QString> results;
    
    // TODO: 实际实现需要查询数据源
    qDebug() << "Query by circle: center=" << center << "radius=" << radius;
    
    return results;
}

QList<QString> SpatialAnalyzer::queryByBuffer(const QPointF &point, double bufferDistance)
{
    QList<QString> results;
    
    // TODO: 实际实现需要查询数据源
    qDebug() << "Query by buffer: point=" << point << "distance=" << bufferDistance;
    
    return results;
}

// ========================================
// 辅助方法
// ========================================

void SpatialAnalyzer::emitProgress(int current, int total, const QString &message)
{
    emit analysisProgress(current, total, message);
}

