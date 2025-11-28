#ifndef SPATIALANALYZER_H
#define SPATIALANALYZER_H

#include <QObject>
#include <QPointF>
#include <QPolygonF>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

/**
 * @brief 空间分析结果结构
 */
struct AnalysisResult {
    bool success;              // 分析是否成功
    QString message;           // 结果消息
    QVariantMap data;          // 结果数据
    QList<QPointF> points;     // 关键点位
    QList<QPolygonF> polygons; // 影响区域
    double score;              // 评分/影响程度
    
    AnalysisResult() : success(false), score(0.0) {}
};

/**
 * @brief 空间查询类型
 */
enum class QueryType {
    PointQuery,      // 点查询
    BoxQuery,        // 矩形框查询
    CircleQuery,     // 圆形查询
    PolygonQuery,    // 多边形查询
    BufferQuery      // 缓冲区查询
};

/**
 * @brief 空间查询参数
 */
struct QueryParams {
    QueryType type;
    QPointF center;         // 中心点
    double radius;          // 半径（米）
    QRectF boundingBox;     // 矩形范围
    QPolygonF polygon;      // 多边形范围
    double bufferDistance;  // 缓冲距离（米）
    QMap<QString, QVariant> filters; // 属性过滤条件
    
    QueryParams() : type(QueryType::PointQuery), radius(0), bufferDistance(0) {}
};

/**
 * @brief 空间分析器基类
 * 
 * 提供空间分析的基础功能，包括距离计算、缓冲区生成等
 */
class SpatialAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit SpatialAnalyzer(QObject *parent = nullptr);
    virtual ~SpatialAnalyzer();

    // ========================================
    // 基础空间计算
    // ========================================
    
    /**
     * @brief 计算两点之间的距离（米）
     * @param lat1, lon1 第一个点的纬度经度
     * @param lat2, lon2 第二个点的纬度经度
     * @return 距离（米）
     */
    static double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    
    /**
     * @brief 计算点到线段的距离
     */
    static double pointToLineDistance(const QPointF &point, const QLineF &line);
    
    /**
     * @brief 判断点是否在多边形内
     */
    static bool isPointInPolygon(const QPointF &point, const QPolygonF &polygon);
    
    /**
     * @brief 生成点的缓冲区（圆形）
     */
    static QPolygonF createCircleBuffer(const QPointF &center, double radius, int segments = 32);
    
    /**
     * @brief 生成线的缓冲区（走廊）
     */
    static QPolygonF createLineBuffer(const QList<QPointF> &line, double width);
    
    /**
     * @brief 计算多边形面积（平方米）
     */
    static double calculatePolygonArea(const QPolygonF &polygon);
    
    /**
     * @brief 计算线段长度（米）
     */
    static double calculateLineLength(const QList<QPointF> &line);

    // ========================================
    // 空间查询
    // ========================================
    
    /**
     * @brief 执行空间查询
     */
    virtual QList<QString> performQuery(const QueryParams &params);
    
    /**
     * @brief 点查询 - 查找指定点附近的对象
     */
    QList<QString> queryByPoint(const QPointF &point, double tolerance);
    
    /**
     * @brief 矩形框查询
     */
    QList<QString> queryByBox(const QRectF &box);
    
    /**
     * @brief 圆形查询
     */
    QList<QString> queryByCircle(const QPointF &center, double radius);
    
    /**
     * @brief 缓冲区查询
     */
    QList<QString> queryByBuffer(const QPointF &point, double bufferDistance);

signals:
    /**
     * @brief 分析进度信号
     * @param current 当前进度
     * @param total 总进度
     * @param message 进度信息
     */
    void analysisProgress(int current, int total, const QString &message);
    
    /**
     * @brief 分析完成信号
     */
    void analysisFinished(const AnalysisResult &result);
    
    /**
     * @brief 分析错误信号
     */
    void analysisError(const QString &error);

protected:
    /**
     * @brief 发送进度信号的辅助方法
     */
    void emitProgress(int current, int total, const QString &message);
    
    /**
     * @brief WGS84坐标转Web Mercator
     */
    static QPointF latLonToMercator(double lat, double lon);
    
    /**
     * @brief Web Mercator转WGS84坐标
     */
    static void mercatorToLatLon(const QPointF &mercator, double &lat, double &lon);

private:
    // 用于存储查询结果缓存
    QMap<QString, QList<QString>> m_queryCache;
    bool m_useCaching;
};

#endif // SPATIALANALYZER_H

