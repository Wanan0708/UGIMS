#ifndef PIPELINERENDERER_H
#define PIPELINERENDERER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QRectF>
#include <QVector>
#include "core/models/pipeline.h"
#include "map/layermanager.h"

class SymbolManager;
class PipelineDAO;
class TileMapManager;

/**
 * @brief 管线渲染器
 * 负责将管线数据渲染到场景中
 */
class PipelineRenderer : public QObject
{
    Q_OBJECT

public:
    explicit PipelineRenderer(QObject *parent = nullptr);
    ~PipelineRenderer();

    // 渲染指定类型的管线
    void renderPipelines(QGraphicsScene *scene, 
                        const QString &pipelineType,
                        const QRectF &bounds = QRectF());
    
    // 渲染单条管线
    QGraphicsPathItem* renderPipeline(QGraphicsScene *scene, 
                                      const Pipeline &pipeline);
    
    // 清除指定类型的管线
    void clear(QGraphicsScene *scene, LayerManager::LayerType type);
    
    // 设置瓦片地图管理器（用于坐标转换）
    void setTileMapManager(TileMapManager *tileMapManager) { m_tileMapManager = tileMapManager; }
    
    // 坐标转换：经纬度 -> 场景坐标
    QPointF geoToScene(const QPointF &geoPoint) const;
    QPointF sceneToGeo(const QPointF &scenePoint) const;
    
    // 设置缩放级别（用于坐标转换）
    void setZoom(int zoom) { m_zoom = zoom; updateTileSize(); }
    int getZoom() const { return m_zoom; }
    
    // 设置瓦片大小
    void setTileSize(int tileSize) { m_tileSize = tileSize; }
    
    // 设置缩放比例（用于调整显示）
    void setScale(qreal scale) { m_scale = scale; }
    qreal getScale() const { return m_scale; }

signals:
    void renderProgress(int current, int total);
    void renderComplete(int count);

private:
    SymbolManager *m_symbolManager;
    PipelineDAO *m_pipelineDao;
    TileMapManager *m_tileMapManager;
    
    // 图形项缓存（按图层类型）
    QHash<LayerManager::LayerType, QList<QGraphicsItem*>> m_itemsCache;
    
    // 缩放比例
    qreal m_scale;
    
    // 瓦片地图参数
    int m_zoom;           // 当前缩放级别
    int m_tileSize;       // 瓦片大小（像素）
    int m_mapWidth;       // 地图总宽度（像素）
    int m_mapHeight;      // 地图总高度（像素）
    
    // 墨卡托投影参数
    static constexpr double EARTH_RADIUS = 6378137.0;  // 地球半径（米）
    static constexpr double ORIGIN_SHIFT = 2.0 * M_PI * EARTH_RADIUS / 2.0;
    
    // 更新地图尺寸（基于当前zoom）
    void updateTileSize();
    
    // 获取图层类型
    LayerManager::LayerType getLayerTypeFromPipelineType(const QString &pipelineType) const;
};

#endif // PIPELINERENDERER_H

