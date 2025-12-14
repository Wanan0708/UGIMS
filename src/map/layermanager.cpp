#include "map/layermanager.h"
#include "map/pipelinerenderer.h"
#include "map/facilityrenderer.h"
#include "map/annotationrenderer.h"
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
    , m_annotationRenderer(nullptr)
{
    // 创建渲染器
    m_pipelineRenderer = new PipelineRenderer(this);
    m_facilityRenderer = new FacilityRenderer(this);
    m_annotationRenderer = new AnnotationRenderer(this);
    
    // 立即设置场景到标注渲染器
    if (m_annotationRenderer && m_scene) {
        m_annotationRenderer->setScene(m_scene);
    }
    
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
    if (m_annotationRenderer) {
        m_annotationRenderer->setTileMapManager(tileMapManager);
        m_annotationRenderer->setScene(m_scene);
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
    m_layerVisibility[Labels] = true;  // 标注默认打开
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
    qDebug() << "[LayerManager] Refreshing layer:" << getLayerName(type);
    
    // 先显示该图层的所有图形项
    QString targetType;
    switch (type) {
    case WaterPipeline:
        targetType = "water_supply";
        break;
    case SewagePipeline:
        targetType = "sewage";
        break;
    case GasPipeline:
        targetType = "gas";
        break;
    case ElectricPipeline:
        targetType = "electric";
        break;
    case TelecomPipeline:
        targetType = "telecom";
        break;
    case HeatPipeline:
        targetType = "heat";
        break;
    case Facilities:
        targetType = "facility";
        break;
    case Labels:
        targetType = "annotation";
        break;
    default:
        break;
    }
    
    // 显示场景中属于该图层的所有图形项
    QList<QGraphicsItem*> allItems = m_scene->items();
    int shownCount = 0;
    
    qDebug() << "[LayerManager] ========== refreshLayer START ==========";
    qDebug() << "[LayerManager] Layer type:" << getLayerName(type) << "targetType:" << targetType;
    qDebug() << "[LayerManager] Total items in scene:" << allItems.size();
    
    for (QGraphicsItem *item : allItems) {
        if (!item) continue;
        
        QString itemType = item->data(0).toString();
        QString itemSubType = item->data(2).toString();
        
        bool shouldShow = false;
        
        if (type == Labels) {
            // 标注图层：显示所有标注（包括背景项）
            shouldShow = (itemType == "annotation");
            // 如果父项是标注，也要显示
            if (!shouldShow && item->parentItem()) {
                QString parentType = item->parentItem()->data(0).toString();
                shouldShow = (parentType == "annotation");
            }
        } else if (type == Facilities) {
            shouldShow = (itemType == "facility");
        } else {
            shouldShow = (itemType == "pipeline" && itemSubType == targetType);
        }
        
        if (shouldShow) {
            item->setVisible(true);
            shownCount++;
            qDebug() << "[LayerManager] ✓ Shown item: type=" << itemType << "subType=" << itemSubType << "visible=" << item->isVisible();
        }
        
        // 调试：输出所有管线项的信息
        if (itemType == "pipeline") {
            qDebug() << "[LayerManager] Pipeline item: subType=" << itemSubType 
                     << "targetType=" << targetType 
                     << "match=" << (itemSubType == targetType)
                     << "visible=" << item->isVisible();
        }
    }
    
    qDebug() << "[LayerManager] Shown" << shownCount << "existing items for layer" << getLayerName(type);
    qDebug() << "[LayerManager] ========== refreshLayer MIDDLE ==========";
    
    // 检查缓存中是否已有项，如果有则只显示它们，不重新渲染
    bool hasCachedItems = false;
    switch (type) {
    case WaterPipeline:
    case SewagePipeline:
    case GasPipeline:
    case ElectricPipeline:
    case TelecomPipeline:
    case HeatPipeline:
        if (m_pipelineRenderer) {
            QList<QGraphicsItem*> cached = m_pipelineRenderer->getCachedItems(type);
            if (!cached.isEmpty()) {
                hasCachedItems = true;
                // 显示缓存中的项
                for (QGraphicsItem *item : cached) {
                    if (item) {
                        item->setVisible(true);
                    }
                }
                qDebug() << "[LayerManager] Shown" << cached.size() << "cached items for layer" << getLayerName(type);
            }
        }
        break;
    case Facilities:
        if (m_facilityRenderer) {
            QList<QGraphicsItem*> cached = m_facilityRenderer->getCachedItems();
            if (!cached.isEmpty()) {
                hasCachedItems = true;
                for (QGraphicsItem *item : cached) {
                    if (item) {
                        item->setVisible(true);
                    }
                }
                qDebug() << "[LayerManager] Shown" << cached.size() << "cached facility items";
            }
        }
        break;
    case Labels:
        // 标注总是重新渲染（因为可能数据有变化）
        hasCachedItems = false;
        break;
    default:
        break;
    }
    
    // 如果缓存中没有项，才重新渲染
    if (!hasCachedItems) {
        qDebug() << "[LayerManager] No cached items, re-rendering layer" << getLayerName(type);
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
            if (m_annotationRenderer && m_scene) {
                // 确保标注渲染器已正确设置场景和瓦片管理器
                if (!m_annotationRenderer->scene()) {
                    m_annotationRenderer->setScene(m_scene);
                }
                if (m_tileMapManager) {
                    m_annotationRenderer->setTileMapManager(m_tileMapManager);
                    // 从tileMapManager获取当前缩放级别
                    int currentZoom = m_tileMapManager->getZoom();
                    qDebug() << "[LayerManager] Setting annotation renderer zoom from tileMapManager:" << currentZoom;
                    m_annotationRenderer->setZoom(currentZoom);
                } else if (m_pipelineRenderer) {
                    // 如果没有tileMapManager，从管线渲染器获取
                    int pipelineZoom = m_pipelineRenderer->getZoom();
                    qDebug() << "[LayerManager] Setting annotation renderer zoom from pipeline renderer:" << pipelineZoom;
                    m_annotationRenderer->setZoom(pipelineZoom);
                } else {
                    // 默认使用10级缩放
                    qDebug() << "[LayerManager] Using default zoom level 10 for annotation renderer";
                    m_annotationRenderer->setZoom(10);
                }
                m_annotationRenderer->renderAllAnnotations(m_visibleBounds);
            }
            break;
        default:
            break;
        }
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
    qDebug() << "[LayerManager] Clearing layer:" << getLayerName(type);
    
    // 先遍历场景中的所有图形项，隐藏属于该图层的项
    QList<QGraphicsItem*> allItems = m_scene->items();
    QString targetType;
    
    // 根据图层类型确定要隐藏的图形项类型
    switch (type) {
    case WaterPipeline:
        targetType = "water_supply";
        break;
    case SewagePipeline:
        targetType = "sewage";
        break;
    case GasPipeline:
        targetType = "gas";
        break;
    case ElectricPipeline:
        targetType = "electric";
        break;
    case TelecomPipeline:
        targetType = "telecom";
        break;
    case HeatPipeline:
        targetType = "heat";
        break;
    case Facilities:
        targetType = "facility";
        break;
    case Labels:
        targetType = "annotation";
        break;
    default:
        return;
    }
    
    int hiddenCount = 0;
    int checkedCount = 0;
    int pipelineCount = 0;
    int facilityCount = 0;
    
    qDebug() << "[LayerManager] ========== clearLayer START ==========";
    qDebug() << "[LayerManager] Layer type:" << getLayerName(type) << "targetType:" << targetType;
    qDebug() << "[LayerManager] Total items in scene:" << allItems.size();
    
    for (QGraphicsItem *item : allItems) {
        if (!item) continue;
        
        QString itemType = item->data(0).toString();  // 实体类型：pipeline, facility, annotation
        QString itemSubType = item->data(2).toString();  // 管线类型或设施类型
        
        // 统计不同类型的项
        if (itemType == "pipeline") pipelineCount++;
        if (itemType == "facility") facilityCount++;
        
        bool shouldHide = false;
        
        if (type == Labels) {
            // 标注图层：隐藏所有标注（包括背景项）
            shouldHide = (itemType == "annotation");
            // 如果父项是标注，也要隐藏
            if (!shouldHide && item->parentItem()) {
                QString parentType = item->parentItem()->data(0).toString();
                shouldHide = (parentType == "annotation");
            }
        } else if (type == Facilities) {
            // 设施图层：隐藏所有设施
            shouldHide = (itemType == "facility");
        } else {
            // 管线图层：隐藏对应类型的管线
            shouldHide = (itemType == "pipeline" && itemSubType == targetType);
        }
        
        if (shouldHide) {
            item->setVisible(false);
            hiddenCount++;
            qDebug() << "[LayerManager] ✓ Hidden item: type=" << itemType << "subType=" << itemSubType << "visible=" << item->isVisible();
        }
        
        // 调试：输出所有管线项的信息
        if (itemType == "pipeline") {
            qDebug() << "[LayerManager] Pipeline item: subType=" << itemSubType 
                     << "targetType=" << targetType 
                     << "match=" << (itemSubType == targetType)
                     << "visible=" << item->isVisible();
        }
        checkedCount++;
    }
    
    qDebug() << "[LayerManager] Statistics: pipelines=" << pipelineCount << "facilities=" << facilityCount;
    qDebug() << "[LayerManager] Checked" << checkedCount << "items, hidden" << hiddenCount << "items for layer" << getLayerName(type);
    qDebug() << "[LayerManager] ========== clearLayer END ==========";
    LOG_DEBUG(QString("Hidden %1 items for layer %2").arg(hiddenCount).arg(getLayerName(type)));
    
    // 然后调用渲染器的 clear 方法（隐藏缓存中的项）
    switch (type) {
    case WaterPipeline:
    case SewagePipeline:
    case GasPipeline:
    case ElectricPipeline:
    case TelecomPipeline:
    case HeatPipeline:
        if (m_pipelineRenderer) {
            m_pipelineRenderer->clear(m_scene, type);
        }
        break;
    case Facilities:
        if (m_facilityRenderer) {
            m_facilityRenderer->clear(m_scene);
        }
        break;
    case Labels:
        if (m_annotationRenderer) {
            m_annotationRenderer->clearAll();
        }
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
    
    if (m_annotationRenderer) {
        m_annotationRenderer->setZoom(zoom);
        // 如果缩放级别变化，可能需要重新渲染标注
        if (isLayerVisible(Labels)) {
            refreshLayer(Labels);
        }
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

