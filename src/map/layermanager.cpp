#include "map/layermanager.h"
#include "map/pipelinerenderer.h"
#include "map/facilityrenderer.h"
#include "tilemap/tilemapmanager.h"
#include "core/common/logger.h"

// 初始化静态成员
QHash<LayerManager::LayerType, QString> LayerManager::s_layerNames = {
    {BaseMap, "底图"},
    {WaterPipeline, "给水管线"},
    {SewagePipeline, "排水管线"},
    {GasPipeline, "燃气管线"},
    {ElectricPipeline, "电力电缆"},
    {TelecomPipeline, "通信光缆"},
    {HeatPipeline, "供热管线"},
    {Facilities, "设施点"},
    {Labels, "标注"}
};

LayerManager::LayerManager(QGraphicsScene *scene, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_tileMapManager(nullptr)
    , m_pipelineRenderer(nullptr)
    , m_facilityRenderer(nullptr)
{
    // 创建渲染器
    m_pipelineRenderer = new PipelineRenderer(this);
    m_facilityRenderer = new FacilityRenderer(this);
    
    // 初始化图层
    initializeLayers();
    
    LOG_INFO("LayerManager initialized");
}

LayerManager::~LayerManager()
{
    clearAllLayers();
}

void LayerManager::setScene(QGraphicsScene *scene)
{
    m_scene = scene;
    LOG_INFO("Scene set for LayerManager");
}

void LayerManager::setTileMapManager(TileMapManager *tileMapManager)
{
    m_tileMapManager = tileMapManager;
    
    // 传递给渲染器
    if (m_pipelineRenderer) {
        m_pipelineRenderer->setTileMapManager(tileMapManager);
    }
    if (m_facilityRenderer) {
        m_facilityRenderer->setTileMapManager(tileMapManager);
    }
}

void LayerManager::initializeLayers()
{
    // 默认所有图层可见
    m_layerVisibility[BaseMap] = true;
    m_layerVisibility[WaterPipeline] = true;
    m_layerVisibility[SewagePipeline] = true;
    m_layerVisibility[GasPipeline] = true;
    m_layerVisibility[ElectricPipeline] = true;
    m_layerVisibility[TelecomPipeline] = true;
    m_layerVisibility[HeatPipeline] = true;
    m_layerVisibility[Facilities] = true;
    m_layerVisibility[Labels] = false;  // 标注默认关闭
}

void LayerManager::setLayerVisible(LayerType type, bool visible)
{
    if (m_layerVisibility.value(type) == visible) {
        return;  // 状态未改变
    }
    
    m_layerVisibility[type] = visible;
    emit layerVisibilityChanged(type, visible);
    
    LOG_INFO(QString("Layer '%1' visibility set to %2")
                 .arg(getLayerName(type))
                 .arg(visible ? "visible" : "hidden"));
    
    // 刷新对应图层
    if (visible) {
        refreshLayer(type);
    } else {
        clearLayer(type);
    }
}

bool LayerManager::isLayerVisible(LayerType type) const
{
    return m_layerVisibility.value(type, false);
}

void LayerManager::toggleLayer(LayerType type)
{
    setLayerVisible(type, !isLayerVisible(type));
}

QList<LayerManager::LayerType> LayerManager::getAllLayerTypes() const
{
    return m_layerVisibility.keys();
}

QString LayerManager::getLayerName(LayerType type) const
{
    return s_layerNames.value(type, "Unknown");
}

void LayerManager::refreshLayer(LayerType type)
{
    if (!m_scene) {
        LOG_WARNING("Cannot refresh layer: scene is null");
        return;
    }
    
    if (!isLayerVisible(type)) {
        return;  // 图层不可见，无需刷新
    }
    
    LOG_INFO(QString("Refreshing layer: %1").arg(getLayerName(type)));
    
    // 根据图层类型调用相应的渲染器
    switch (type) {
    case WaterPipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "water_supply", m_visibleBounds);
        break;
    case SewagePipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "sewage", m_visibleBounds);
        break;
    case GasPipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "gas", m_visibleBounds);
        break;
    case ElectricPipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "electric", m_visibleBounds);
        break;
    case TelecomPipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "telecom", m_visibleBounds);
        break;
    case HeatPipeline:
        m_pipelineRenderer->renderPipelines(m_scene, "heat", m_visibleBounds);
        break;
    case Facilities:
        m_facilityRenderer->renderFacilities(m_scene, m_visibleBounds);
        break;
    case Labels:
        // TODO: 实现标注渲染
        break;
    default:
        break;
    }
    
    emit layerRefreshed(type);
}

void LayerManager::refreshAllLayers()
{
    LOG_INFO("Refreshing all visible layers");
    
    for (LayerType type : m_layerVisibility.keys()) {
        if (isLayerVisible(type) && type != BaseMap) {
            refreshLayer(type);
        }
    }
}

void LayerManager::clearLayer(LayerType type)
{
    if (!m_scene) {
        return;
    }
    
    LOG_DEBUG(QString("Clearing layer: %1").arg(getLayerName(type)));
    
    // 清除对应图层的图形项
    // 注意：这里需要根据图层类型选择性删除图形项
    // 当前简化实现，后续可以通过给图形项添加标签来精确删除
    
    switch (type) {
    case WaterPipeline:
    case SewagePipeline:
    case GasPipeline:
    case ElectricPipeline:
    case TelecomPipeline:
    case HeatPipeline:
        m_pipelineRenderer->clear(m_scene, type);
        break;
    case Facilities:
        m_facilityRenderer->clear(m_scene);
        break;
    default:
        break;
    }
}

void LayerManager::clearAllLayers()
{
    LOG_INFO("Clearing all layers");
    
    for (LayerType type : m_layerVisibility.keys()) {
        if (type != BaseMap) {
            clearLayer(type);
        }
    }
}

void LayerManager::setVisibleBounds(const QRectF &bounds)
{
    m_visibleBounds = bounds;
    LOG_DEBUG(QString("Visible bounds set: [%1, %2, %3, %4]")
                  .arg(bounds.left()).arg(bounds.top())
                  .arg(bounds.width()).arg(bounds.height()));
}

void LayerManager::onDataChanged()
{
    LOG_INFO("Data changed, refreshing all layers");
    refreshAllLayers();
}

void LayerManager::setZoom(int zoom)
{
    // 同步缩放级别到所有渲染器
    if (m_pipelineRenderer) {
        m_pipelineRenderer->setZoom(zoom);
        LOG_DEBUG(QString("Pipeline renderer zoom set to %1").arg(zoom));
    }
    
    if (m_facilityRenderer) {
        m_facilityRenderer->setZoom(zoom);
        LOG_DEBUG(QString("Facility renderer zoom set to %1").arg(zoom));
    }
}

void LayerManager::setTileSize(int tileSize)
{
    // 同步瓦片大小到所有渲染器
    if (m_pipelineRenderer) {
        m_pipelineRenderer->setTileSize(tileSize);
    }
    
    if (m_facilityRenderer) {
        m_facilityRenderer->setTileSize(tileSize);
    }
}

