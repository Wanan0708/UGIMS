#include "map/facilityrenderer.h"
#include "map/symbolmanager.h"
#include "map/pipelinerenderer.h"
#include "dao/facilitydao.h"
#include "tilemap/tilemapmanager.h"
#include "core/common/logger.h"
#include <QPen>
#include <QBrush>
#include <QtMath>

FacilityRenderer::FacilityRenderer(QObject *parent)
    : QObject(parent)
    , m_symbolManager(new SymbolManager(this))
    , m_facilityDao(new FacilityDAO())
    , m_tileMapManager(nullptr)
    , m_zoom(10)          // 默认缩放级别
    , m_tileSize(256)     // 默认瓦片大小
    , m_mapWidth(0)
    , m_mapHeight(0)
{
    updateTileSize();
    LOG_INFO("FacilityRenderer initialized");
}

FacilityRenderer::~FacilityRenderer()
{
    delete m_facilityDao;
}

void FacilityRenderer::renderFacilities(QGraphicsScene *scene, const QRectF &bounds)
{
    if (!scene) {
        LOG_WARNING("Cannot render facilities: scene is null");
        return;
    }
    
    LOG_INFO("Rendering facilities");
    
    // 1. 从数据库加载设施数据
    QVector<Facility> facilities;
    
    if (bounds.isValid() && !bounds.isEmpty()) {
        // 按边界框查询
        facilities = m_facilityDao->findByBounds(bounds);
        LOG_INFO(QString("Loaded %1 facilities in bounds").arg(facilities.size()));
    } else {
        // 查询所有设施
        facilities = m_facilityDao->findAll(1000);
        LOG_INFO(QString("Loaded %1 facilities").arg(facilities.size()));
    }
    
    if (facilities.isEmpty()) {
        LOG_WARNING("No facilities found");
        return;
    }
    
    // 2. 渲染每个设施
    int rendered = 0;
    for (int i = 0; i < facilities.size(); i++) {
        const Facility &facility = facilities[i];
        
        QGraphicsEllipseItem *item = renderFacility(scene, facility);
        if (item) {
            m_itemsCache.append(item);
            rendered++;
        }
        
        // 发送进度信号
        if (i % 10 == 0) {
            emit renderProgress(i + 1, facilities.size());
        }
    }
    
    emit renderProgress(facilities.size(), facilities.size());
    emit renderComplete(rendered);
    
    LOG_INFO(QString("Rendered %1 facilities successfully").arg(rendered));
}

void FacilityRenderer::renderFacilitiesByType(QGraphicsScene *scene,
                                              const QString &facilityType,
                                              const QRectF &bounds)
{
    if (!scene) {
        LOG_WARNING("Cannot render facilities: scene is null");
        return;
    }
    
    LOG_INFO(QString("Rendering facilities of type: %1").arg(facilityType));
    
    // 1. 从数据库加载指定类型的设施
    QVector<Facility> facilities;
    
    if (bounds.isValid() && !bounds.isEmpty()) {
        // 先按边界查询，再过滤类型
        facilities = m_facilityDao->findByBounds(bounds);
    } else {
        facilities = m_facilityDao->findByType(facilityType);
    }
    
    // 2. 渲染
    int rendered = 0;
    for (const Facility &facility : facilities) {
        if (facility.facilityType() == facilityType) {
            QGraphicsEllipseItem *item = renderFacility(scene, facility);
            if (item) {
                m_itemsCache.append(item);
                rendered++;
            }
        }
    }
    
    emit renderComplete(rendered);
    LOG_INFO(QString("Rendered %1 facilities of type %2")
                 .arg(rendered).arg(facilityType));
}

QGraphicsEllipseItem* FacilityRenderer::renderFacility(QGraphicsScene *scene,
                                                       const Facility &facility)
{
    if (!scene || !facility.isValid()) {
        return nullptr;
    }
    
    // 检查坐标是否有效
    QPointF geoCoord = facility.coordinate();
    if (geoCoord.isNull()) {
        LOG_WARNING(QString("Facility %1 has null coordinates")
                        .arg(facility.facilityId()));
        return nullptr;
    }
    
    // 1. 转换坐标
    QPointF scenePos = geoToScene(geoCoord);
    
    // 2. 获取样式
    int size = m_symbolManager->getFacilityIconSize(facility.facilityType());
    QBrush brush = m_symbolManager->getFacilityBrush(facility.facilityType());
    QPen pen(Qt::black, 1.5);
    
    // 根据健康度调整外框颜色
    if (facility.healthScore() < 60) {
        pen.setColor(Qt::red);
        pen.setWidth(2);
    } else if (facility.healthScore() < 80) {
        pen.setColor(Qt::darkYellow);
    }
    
    // 3. 创建圆形图标
    qreal radius = size / 2.0;
    QGraphicsEllipseItem *item = scene->addEllipse(
        scenePos.x() - radius,
        scenePos.y() - radius,
        size,
        size,
        pen,
        brush
    );
    
    // 4. 设置数据
    item->setData(0, "facility");  // 类型标记
    item->setData(1, facility.id());  // 数据库ID
    item->setData(2, facility.facilityId());  // 设施编号
    item->setData(3, facility.facilityType());  // 设施类型
    
    // 5. 设置工具提示
    QString tooltip = QString("%1\n类型: %2\n规格: %3\n健康度: %4分")
                          .arg(facility.getDisplayName())
                          .arg(facility.getTypeDisplayName())
                          .arg(facility.spec())
                          .arg(facility.healthScore());
    
    if (!facility.pipelineId().isEmpty()) {
        tooltip += QString("\n关联管线: %1").arg(facility.pipelineId());
    }
    
    item->setToolTip(tooltip);
    
    // 6. 设置Z值（确保在管线之上）
    item->setZValue(20);
    
    return item;
}

void FacilityRenderer::clear(QGraphicsScene *scene)
{
    if (!scene) {
        return;
    }
    
    // 删除所有缓存的图形项
    for (QGraphicsItem *item : m_itemsCache) {
        scene->removeItem(item);
        delete item;
    }
    
    m_itemsCache.clear();
    
    LOG_DEBUG("Cleared all facility items");
}

void FacilityRenderer::updateTileSize()
{
    // 更新地图总尺寸（基于当前缩放级别）
    int numTiles = 1 << m_zoom;  // 2^zoom
    m_mapWidth = numTiles * m_tileSize;
    m_mapHeight = numTiles * m_tileSize;
    
    LOG_INFO(QString("Facility renderer map size updated: zoom=%1, size=%2x%3")
             .arg(m_zoom).arg(m_mapWidth).arg(m_mapHeight));
}

QPointF FacilityRenderer::geoToScene(const QPointF &geoPoint) const
{
    // 使用 TileMapManager 的坐标转换（与瓦片布局完全一致）
    if (m_tileMapManager) {
        return m_tileMapManager->geoToScene(geoPoint.x(), geoPoint.y());
    }
    
    // 降级方案：如果没有 TileMapManager，使用简单的绝对坐标
    double lon = geoPoint.x();
    double lat = geoPoint.y();
    lat = qBound(-85.05112878, lat, 85.05112878);
    
    int n = 1 << m_zoom;
    double tileX = (lon + 180.0) / 360.0 * n;
    double latRad = lat * M_PI / 180.0;
    double tileY = (1.0 - qLn(qTan(latRad) + 1.0 / qCos(latRad)) / M_PI) / 2.0 * n;
    
    double sceneX = tileX * m_tileSize;
    double sceneY = tileY * m_tileSize;
    
    return QPointF(sceneX, sceneY);
}

