#ifndef ANNOTATIONRENDERER_H
#define ANNOTATIONRENDERER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QHash>
#include <QRectF>
#include "core/models/pipeline.h"
#include "core/models/facility.h"

class TileMapManager;
class PipelineDAO;
class FacilityDAO;

/**
 * @brief 标注渲染器
 * 负责在地图上渲染管线编号、设施名称等标注信息
 */
class AnnotationRenderer : public QObject
{
    Q_OBJECT

public:
    explicit AnnotationRenderer(QObject *parent = nullptr);
    ~AnnotationRenderer();

    // 设置场景
    void setScene(QGraphicsScene *scene);
    QGraphicsScene* scene() const { return m_scene; }
    
    // 设置瓦片地图管理器（用于坐标转换）
    void setTileMapManager(TileMapManager *tileMapManager);

    // 渲染所有标注
    void renderAllAnnotations(const QRectF &bounds = QRectF());
    
    // 渲染管线标注
    void renderPipelineAnnotations(const QString &pipelineType = QString(), const QRectF &bounds = QRectF());
    
    // 渲染设施标注
    void renderFacilityAnnotations(const QRectF &bounds = QRectF());
    
    // 清除所有标注
    void clearAll();
    
    // 清除指定类型的标注
    void clearPipelineAnnotations();
    void clearFacilityAnnotations();
    
    // 设置标注可见性
    void setPipelineLabelsVisible(bool visible);
    void setFacilityLabelsVisible(bool visible);
    
    // 设置标注样式
    void setLabelFont(const QFont &font);
    void setLabelColor(const QColor &color);
    void setLabelBackgroundColor(const QColor &color);
    
    // 设置最小缩放级别（低于此级别不显示标注）
    void setMinZoomLevel(int zoom) { m_minZoomLevel = zoom; }
    int minZoomLevel() const { return m_minZoomLevel; }
    
    // 设置当前缩放级别
    void setZoom(int zoom) { m_zoom = zoom; }
    int zoom() const { return m_zoom; }

signals:
    void renderProgress(int current, int total);
    void renderComplete(int count);

private:
    // 创建管线标注
    QGraphicsTextItem* createPipelineLabel(const Pipeline &pipeline, const QPointF &scenePos);
    
    // 创建设施标注
    QGraphicsTextItem* createFacilityLabel(const Facility &facility, const QPointF &scenePos);
    
    // 坐标转换
    QPointF geoToScene(const QPointF &geoPoint) const;
    
    // 计算标注位置（避免重叠）
    QPointF calculateLabelPosition(const QPointF &basePos, const QSizeF &labelSize) const;
    
    // 检查标注是否在可视范围内
    bool isInBounds(const QPointF &pos, const QRectF &bounds) const;

private:
    QGraphicsScene *m_scene;
    TileMapManager *m_tileMapManager;
    PipelineDAO *m_pipelineDao;
    FacilityDAO *m_facilityDao;
    
    // 标注项缓存
    QList<QGraphicsTextItem*> m_pipelineLabels;
    QList<QGraphicsTextItem*> m_facilityLabels;
    
    // 标注样式
    QFont m_labelFont;
    QColor m_labelColor;
    QColor m_labelBackgroundColor;
    
    // 可见性控制
    bool m_pipelineLabelsVisible;
    bool m_facilityLabelsVisible;
    
    // 缩放级别控制
    int m_minZoomLevel;  // 最小显示缩放级别
    int m_zoom;          // 当前缩放级别
};

#endif // ANNOTATIONRENDERER_H

