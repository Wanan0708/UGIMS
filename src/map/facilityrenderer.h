#ifndef FACILITYRENDERER_H
#define FACILITYRENDERER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QRectF>
#include <QVector>
#include "core/models/facility.h"

class SymbolManager;
class FacilityDAO;
class TileMapManager;

/**
 * @brief 设施渲染器
 * 负责将设施数据渲染到场景中
 */
class FacilityRenderer : public QObject
{
    Q_OBJECT

public:
    explicit FacilityRenderer(QObject *parent = nullptr);
    ~FacilityRenderer();

    // 渲染所有设施
    void renderFacilities(QGraphicsScene *scene, const QRectF &bounds = QRectF());
    
    // 渲染指定类型的设施
    void renderFacilitiesByType(QGraphicsScene *scene, 
                               const QString &facilityType,
                               const QRectF &bounds = QRectF());
    
    // 渲染单个设施
    QGraphicsEllipseItem* renderFacility(QGraphicsScene *scene, 
                                        const Facility &facility);
    
    // 清除所有设施
    void clear(QGraphicsScene *scene);
    
    // 设置瓦片地图管理器（用于坐标转换）
    void setTileMapManager(TileMapManager *tileMapManager) { m_tileMapManager = tileMapManager; }
    
    // 设置缩放级别（用于坐标转换）
    void setZoom(int zoom) { m_zoom = zoom; updateTileSize(); }
    int getZoom() const { return m_zoom; }
    
    // 设置瓦片大小
    void setTileSize(int tileSize) { m_tileSize = tileSize; }

signals:
    void renderProgress(int current, int total);
    void renderComplete(int count);

private:
    SymbolManager *m_symbolManager;
    FacilityDAO *m_facilityDao;
    TileMapManager *m_tileMapManager;
    
    // 图形项缓存
    QList<QGraphicsItem*> m_itemsCache;
    
    // 瓦片地图参数
    int m_zoom;           // 当前缩放级别
    int m_tileSize;       // 瓦片大小（像素）
    int m_mapWidth;       // 地图总宽度（像素）
    int m_mapHeight;      // 地图总高度（像素）
    
    // 更新地图尺寸
    void updateTileSize();
    
    // 坐标转换
    QPointF geoToScene(const QPointF &geoPoint) const;
};

#endif // FACILITYRENDERER_H

