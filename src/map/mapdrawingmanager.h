#ifndef MAPDRAWINGMANAGER_H
#define MAPDRAWINGMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <QVector>
#include <QPointF>
#include <QPen>
#include "tilemap/tilemapmanager.h"

/**
 * @brief 地图绘制管理器
 * 
 * 负责管理地图上的绘制操作，支持：
 * - 绘制管线（折线）
 * - 绘制设施（点）
 * - 临时预览（橡皮筋效果）
 * - 生成 WKT 几何数据
 */
class MapDrawingManager : public QObject
{
    Q_OBJECT

public:
    // 绘制模式枚举
    enum DrawingMode {
        NoDrawing,          // 无绘制
        DrawingPolyline,    // 绘制折线（管线）
        DrawingPoint        // 绘制点（设施）
    };
    Q_ENUM(DrawingMode)

    explicit MapDrawingManager(QGraphicsScene *scene, 
                              QGraphicsView *view,
                              TileMapManager *tileManager,
                              QObject *parent = nullptr);
    ~MapDrawingManager();

    /**
     * @brief 开始绘制管线
     * @param pipelineType 管线类型标识
     */
    void startDrawingPipeline(const QString &pipelineType);
    
    /**
     * @brief 开始绘制设施
     * @param facilityType 设施类型标识
     */
    void startDrawingFacility(const QString &facilityType);
    
    /**
     * @brief 取消当前绘制
     */
    void cancelDrawing();
    
    /**
     * @brief 完成当前绘制
     */
    void finishDrawing();
    
    /**
     * @brief 获取当前绘制模式
     */
    DrawingMode currentMode() const { return m_mode; }
    
    /**
     * @brief 是否正在绘制
     */
    bool isDrawing() const { return m_mode != NoDrawing; }
    
    /**
     * @brief 处理鼠标点击事件
     */
    void handleMouseClick(const QPointF &scenePos);
    
    /**
     * @brief 处理鼠标移动事件
     */
    void handleMouseMove(const QPointF &scenePos);
    
    /**
     * @brief 处理右键点击事件（完成绘制）
     */
    void handleRightClick(const QPointF &scenePos);

signals:
    /**
     * @brief 管线绘制完成信号
     * @param pipelineType 管线类型
     * @param wkt 几何数据（WKT格式）
     * @param points 场景坐标点列表
     */
    void pipelineDrawingFinished(const QString &pipelineType, 
                                 const QString &wkt, 
                                 const QVector<QPointF> &points);
    
    /**
     * @brief 设施绘制完成信号
     * @param facilityType 设施类型
     * @param wkt 几何数据（WKT格式）
     * @param point 场景坐标点
     */
    void facilityDrawingFinished(const QString &facilityType, 
                                const QString &wkt, 
                                const QPointF &point);
    
    /**
     * @brief 绘制状态改变信号
     * @param isDrawing 是否正在绘制
     */
    void drawingStateChanged(bool isDrawing);

private:
    void clearTemporaryGraphics();
    void updatePreviewLine(const QPointF &currentPos);
    void createPreviewPoint(const QPointF &pos);
    QString generateLineStringWKT(const QVector<QPointF> &points);
    QString generatePointWKT(const QPointF &point);
    QPointF sceneToGeo(const QPointF &scenePos);
    QCursor createPipelineCursor(const QString &pipelineType);  // 创建管线类型光标
    void updateCursor();  // 更新鼠标样式
    
    // 核心组件
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    TileMapManager *m_tileManager;
    
    // 绘制状态
    DrawingMode m_mode;
    QString m_currentType;              // 当前类型标识
    QVector<QPointF> m_points;          // 已绘制的点列表
    
    // 临时图形项
    QVector<QGraphicsEllipseItem*> m_pointMarkers;  // 点标记
    QGraphicsPathItem *m_previewLine;               // 预览线
    QGraphicsEllipseItem *m_previewPoint;           // 预览点
    
    // 样式配置
    static const int POINT_MARKER_SIZE = 8;         // 点标记大小
    static const int PREVIEW_POINT_SIZE = 12;       // 预览点大小
};

#endif // MAPDRAWINGMANAGER_H
