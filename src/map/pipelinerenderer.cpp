#include "map/pipelinerenderer.h"
#include "map/symbolmanager.h"
#include "dao/pipelinedao.h"
#include "tilemap/tilemapmanager.h"
#include "core/common/logger.h"
#include <QPainterPath>
#include <QtMath>
#include <cmath>

PipelineRenderer::PipelineRenderer(QObject *parent)
    : QObject(parent)
    , m_symbolManager(new SymbolManager(this))
    , m_pipelineDao(new PipelineDAO())
    , m_tileMapManager(nullptr)
    , m_scale(1.0)
    , m_zoom(10)          // 默认缩放级别
    , m_tileSize(256)     // 默认瓦片大小
    , m_mapWidth(0)
    , m_mapHeight(0)
{
    updateTileSize();
    LOG_INFO("PipelineRenderer initialized");
}

PipelineRenderer::~PipelineRenderer()
{
    delete m_pipelineDao;
}

void PipelineRenderer::renderPipelines(QGraphicsScene *scene, 
                                      const QString &pipelineType,
                                      const QRectF &bounds)
{
    if (!scene) {
        LOG_WARNING("Cannot render pipelines: scene is null");
        return;
    }
    
    LOG_INFO(QString("Rendering pipelines of type: %1").arg(pipelineType));
    qDebug() << "[PipelineRenderer] Type:" << pipelineType;
    qDebug() << "[PipelineRenderer] Bounds:" << bounds;
    qDebug() << "[PipelineRenderer] TileMapManager:" << (m_tileMapManager ? "SET" : "NULL");
    qDebug() << "[PipelineRenderer] Current zoom:" << m_zoom;
    
    // 1. 从数据库加载管线数据
    QVector<Pipeline> pipelines;
    
    // 临时：先不使用 bounds 查询，直接查询所有类型的管线来测试
    qDebug() << "[PipelineRenderer] Querying by type (ignore bounds for now)...";
    pipelines = m_pipelineDao->findByType(pipelineType, 1000);
    LOG_INFO(QString("Loaded %1 pipelines of type %2")
                 .arg(pipelines.size()).arg(pipelineType));
    qDebug() << "[PipelineRenderer] Found" << pipelines.size() << "pipelines of type" << pipelineType;
    
    if (pipelines.isEmpty()) {
        LOG_WARNING(QString("No pipelines found for type: %1").arg(pipelineType));
        qDebug() << "[PipelineRenderer] ⚠️  No data found in database!";
        return;
    }
    
    // 2. 渲染每条管线
    int rendered = 0;
    LayerManager::LayerType layerType = getLayerTypeFromPipelineType(pipelineType);
    
    for (int i = 0; i < pipelines.size(); i++) {
        const Pipeline &pipeline = pipelines[i];
        
        // 只渲染指定类型的管线
        if (pipeline.pipelineType() == pipelineType) {
            QGraphicsPathItem *item = renderPipeline(scene, pipeline);
            if (item) {
                // 缓存图形项
                m_itemsCache[layerType].append(item);
                rendered++;
            }
        }
        
        // 发送进度信号
        if (i % 10 == 0) {
            emit renderProgress(i + 1, pipelines.size());
        }
    }
    
    emit renderProgress(pipelines.size(), pipelines.size());
    emit renderComplete(rendered);
    
    LOG_INFO(QString("Rendered %1 pipelines successfully").arg(rendered));
}

QGraphicsPathItem* PipelineRenderer::renderPipeline(QGraphicsScene *scene, 
                                                    const Pipeline &pipeline)
{
    if (!scene || !pipeline.isValid()) {
        return nullptr;
    }
    
    // 获取管线坐标
    QVector<QPointF> coords = pipeline.coordinates();
    if (coords.size() < 2) {
        LOG_WARNING(QString("Pipeline %1 has insufficient coordinates")
                        .arg(pipeline.pipelineId()));
        return nullptr;
    }
    
    // 1. 创建路径
    QPainterPath path;
    QPointF firstPoint = geoToScene(coords[0]);
    path.moveTo(firstPoint);
    
    // 调试：输出第一个坐标的转换结果（只输出第一条管线）
    static bool firstPipeline = true;
    if (firstPipeline) {
        qDebug() << "[PipelineRenderer] Pipeline" << pipeline.pipelineId() 
                 << "geo:" << coords[0] << "-> scene:" << firstPoint;
        firstPipeline = false;
    }
    
    for (int i = 1; i < coords.size(); i++) {
        QPointF scenePoint = geoToScene(coords[i]);
        path.lineTo(scenePoint);
    }
    
    // 2. 获取样式
    QPen pen = m_symbolManager->getPipelinePen(
        pipeline.pipelineType(),
        pipeline.diameterMm()
    );
    
    // 根据健康度调整透明度
    if (pipeline.healthScore() < 60) {
        QColor color = pen.color();
        color.setAlpha(180);  // 不健康的管线半透明
        pen.setColor(color);
    }
    
    // 3. 创建图形项并添加到场景
    QGraphicsPathItem *item = scene->addPath(path, pen);
    
    // 4. 设置数据（用于后续查询和删除）
    item->setData(0, "pipeline");  // 类型标记
    item->setData(1, pipeline.id());  // 数据库ID
    item->setData(2, pipeline.pipelineId());  // 管线编号
    item->setData(3, pipeline.pipelineType());  // 管线类型
    
    // 5. 设置工具提示
    QString tooltip = QString("%1\n类型: %2\n管径: DN%3\n健康度: %4分")
                          .arg(pipeline.getDisplayName())
                          .arg(pipeline.getTypeDisplayName())
                          .arg(pipeline.diameterMm())
                          .arg(pipeline.healthScore());
    item->setToolTip(tooltip);
    
    // 6. 设置Z值（确保在底图之上）
    item->setZValue(10);
    
    return item;
}

void PipelineRenderer::clear(QGraphicsScene *scene, LayerManager::LayerType type)
{
    if (!scene) {
        return;
    }
    
    // 从缓存中删除图形项
    QList<QGraphicsItem*> items = m_itemsCache.value(type);
    for (QGraphicsItem *item : items) {
        scene->removeItem(item);
        delete item;
    }
    
    m_itemsCache[type].clear();
    
    LOG_DEBUG(QString("Cleared %1 pipeline items").arg(items.size()));
}

void PipelineRenderer::updateTileSize()
{
    // 更新地图总尺寸（基于当前缩放级别）
    int numTiles = 1 << m_zoom;  // 2^zoom
    m_mapWidth = numTiles * m_tileSize;
    m_mapHeight = numTiles * m_tileSize;
    
    LOG_INFO(QString("Pipeline renderer map size updated: zoom=%1, size=%2x%3")
             .arg(m_zoom).arg(m_mapWidth).arg(m_mapHeight));
}

QPointF PipelineRenderer::geoToScene(const QPointF &geoPoint) const
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

QPointF PipelineRenderer::sceneToGeo(const QPointF &scenePoint) const
{
    // 场景像素坐标 -> 瓦片坐标 -> 经纬度
    
    // 场景坐标 -> 瓦片坐标
    double tileX = scenePoint.x() / m_tileSize;
    double tileY = scenePoint.y() / m_tileSize;
    
    int n = 1 << m_zoom;  // 2^zoom
    
    // 瓦片X坐标 -> 经度
    double lon = tileX / n * 360.0 - 180.0;
    
    // 瓦片Y坐标 -> 纬度
    double latRad = qAtan(sinh(M_PI * (1.0 - 2.0 * tileY / n)));
    double lat = latRad * 180.0 / M_PI;
    
    return QPointF(lon, lat);
}

LayerManager::LayerType PipelineRenderer::getLayerTypeFromPipelineType(const QString &pipelineType) const
{
    if (pipelineType == "water_supply") {
        return LayerManager::WaterPipeline;
    } else if (pipelineType == "sewage") {
        return LayerManager::SewagePipeline;
    } else if (pipelineType == "gas") {
        return LayerManager::GasPipeline;
    } else if (pipelineType == "electric") {
        return LayerManager::ElectricPipeline;
    } else if (pipelineType == "telecom") {
        return LayerManager::TelecomPipeline;
    } else if (pipelineType == "heat") {
        return LayerManager::HeatPipeline;
    }
    
    return LayerManager::WaterPipeline;  // 默认
}

