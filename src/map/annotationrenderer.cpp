#include "map/annotationrenderer.h"
#include "tilemap/tilemapmanager.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/common/logger.h"
#include <QFont>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsRectItem>
#include <QtMath>
#include <QDebug>

AnnotationRenderer::AnnotationRenderer(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_tileMapManager(nullptr)
    , m_pipelineDao(new PipelineDAO())
    , m_facilityDao(new FacilityDAO())
    , m_labelFont("Arial", 10)
    , m_labelColor(Qt::black)
    , m_labelBackgroundColor(Qt::white)
    , m_pipelineLabelsVisible(true)
    , m_facilityLabelsVisible(true)
    , m_minZoomLevel(1)  // 默认1级缩放以上就显示标注（允许在任何缩放级别显示）
    , m_zoom(10)
{
    LOG_INFO("AnnotationRenderer initialized");
}

AnnotationRenderer::~AnnotationRenderer()
{
    clearAll();
    delete m_pipelineDao;
    delete m_facilityDao;
}

void AnnotationRenderer::setScene(QGraphicsScene *scene)
{
    m_scene = scene;
}

void AnnotationRenderer::setTileMapManager(TileMapManager *tileMapManager)
{
    m_tileMapManager = tileMapManager;
}

void AnnotationRenderer::renderAllAnnotations(const QRectF &bounds)
{
    qDebug() << "[AnnotationRenderer] renderAllAnnotations called, scene:" << (m_scene ? "SET" : "NULL")
             << "tileMapManager:" << (m_tileMapManager ? "SET" : "NULL")
             << "zoom:" << m_zoom << "minZoom:" << m_minZoomLevel
             << "bounds:" << bounds;
    
    if (!m_scene) {
        LOG_WARNING("Cannot render annotations: scene is null");
        qDebug() << "[AnnotationRenderer] ⚠️  Scene is null, cannot render";
        return;
    }
    
    if (!m_tileMapManager) {
        LOG_WARNING("Cannot render annotations: tileMapManager is null");
        qDebug() << "[AnnotationRenderer] ⚠️  TileMapManager is null, cannot render";
        return;
    }
    
    // 检查缩放级别
    if (m_zoom < m_minZoomLevel) {
        qDebug() << "[AnnotationRenderer] Zoom level" << m_zoom << "is below minimum" << m_minZoomLevel;
        clearAll();
        return;
    }
    
    qDebug() << "[AnnotationRenderer] Starting to render annotations...";
    // 暂时禁用bounds检查，因为坐标系统可能不一致
    renderPipelineAnnotations(QString(), QRectF());
    renderFacilityAnnotations(QRectF());
    qDebug() << "[AnnotationRenderer] Annotation rendering completed";
}

void AnnotationRenderer::renderPipelineAnnotations(const QString &pipelineType, const QRectF &bounds)
{
    if (!m_scene || !m_pipelineLabelsVisible) {
        qDebug() << "[AnnotationRenderer] Skipping pipeline annotations: scene=" << (m_scene ? "SET" : "NULL")
                 << "visible=" << m_pipelineLabelsVisible;
        return;
    }
    
    // 检查缩放级别
    if (m_zoom < m_minZoomLevel) {
        clearPipelineAnnotations();
        return;
    }
    
    // 清除现有标注
    clearPipelineAnnotations();
    
    // 从场景中查找已有的管线图形项，使用它们的场景坐标
    QList<QGraphicsItem*> allItems = m_scene->items();
    QHash<QString, QGraphicsPathItem*> pipelineItems;  // pipelineId -> item
    
    for (QGraphicsItem *item : allItems) {
        if (item->data(0).toString() == "pipeline") {
            QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item);
            if (pathItem) {
                QString pipelineId = item->data(1).toString();
                QString itemPipelineType = item->data(2).toString();
                
                // 如果指定了管线类型，只处理匹配的类型
                if (!pipelineType.isEmpty() && itemPipelineType != pipelineType) {
                    continue;
                }
                
                if (!pipelineId.isEmpty()) {
                    pipelineItems[pipelineId] = pathItem;
                }
            }
        }
    }
    
    qDebug() << "[AnnotationRenderer] Found" << pipelineItems.size() << "pipeline items in scene to label";
    
    if (pipelineItems.isEmpty()) {
        qDebug() << "[AnnotationRenderer] No pipeline items found in scene, skipping labels";
        return;
    }
    
    // 从数据库加载管线数据以获取名称
    QVector<Pipeline> pipelines;
    if (pipelineType.isEmpty()) {
        QStringList types = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
        for (const QString &type : types) {
            QVector<Pipeline> typePipelines = m_pipelineDao->findByType(type, 1000);
            pipelines.append(typePipelines);
        }
    } else {
        pipelines = m_pipelineDao->findByType(pipelineType, 1000);
    }
    
    // 创建管线ID到Pipeline对象的映射
    QHash<QString, Pipeline> pipelineMap;
    for (const Pipeline &p : pipelines) {
        pipelineMap[p.pipelineId()] = p;
    }
    
    int rendered = 0;
    int skippedNoName = 0;
    
    // 遍历场景中的管线项，为每个项创建标注
    for (auto it = pipelineItems.constBegin(); it != pipelineItems.constEnd(); ++it) {
        QString pipelineId = it.key();
        QGraphicsPathItem *pathItem = it.value();
        
        // 获取管线数据
        Pipeline pipeline = pipelineMap.value(pipelineId);
        
        // 检查管线是否有名称或编号
        QString labelText = pipeline.isValid() && !pipeline.pipelineName().isEmpty() ? 
                           pipeline.pipelineName() : 
                           pipelineId;
        if (labelText.isEmpty()) {
            skippedNoName++;
            continue;
        }
        
        // 获取管线的场景坐标（使用路径的中点）
        QPainterPath path = pathItem->path();
        QRectF pathBounds = path.boundingRect();
        QPointF scenePos = pathBounds.center() + pathItem->pos();
        
        qDebug() << "[AnnotationRenderer] Pipeline" << labelText << "scenePos:" << scenePos;
        
        // 创建标注
        QGraphicsTextItem *label = nullptr;
        if (pipeline.isValid()) {
            label = createPipelineLabel(pipeline, scenePos);
        } else {
            // 如果没有找到管线数据，创建一个简单的标注
            label = new QGraphicsTextItem(labelText);
            label->setFont(m_labelFont);
            label->setDefaultTextColor(m_labelColor);
            QRectF textRect = label->boundingRect();
            QPointF labelPos = calculateLabelPosition(scenePos, textRect.size());
            label->setPos(labelPos);
            label->setData(0, "annotation");
            label->setData(1, "pipeline");
            label->setData(2, pipelineId);
            m_scene->addItem(label);
        }
        
        if (label) {
            m_pipelineLabels.append(label);
            rendered++;
            qDebug() << "[AnnotationRenderer] ✓ Created label for pipeline:" << labelText;
        }
    }
    
    qDebug() << "[AnnotationRenderer] Pipeline labels summary: total=" << pipelineItems.size()
             << "rendered=" << rendered
             << "skipped(no name)=" << skippedNoName;
    
    emit renderProgress(pipelines.size(), pipelines.size());
    emit renderComplete(rendered);
    
    LOG_INFO(QString("Rendered %1 pipeline labels").arg(rendered));
    qDebug() << "[AnnotationRenderer] ✅ Rendered" << rendered << "pipeline labels";
}

void AnnotationRenderer::renderFacilityAnnotations(const QRectF &bounds)
{
    if (!m_scene || !m_facilityLabelsVisible) {
        return;
    }
    
    // 检查缩放级别
    if (m_zoom < m_minZoomLevel) {
        clearFacilityAnnotations();
        return;
    }
    
    // 清除现有标注
    clearFacilityAnnotations();
    
    // 从场景中查找已有的设施图形项，使用它们的场景坐标
    QList<QGraphicsItem*> allItems = m_scene->items();
    QHash<QString, QGraphicsEllipseItem*> facilityItems;  // facilityId -> item
    
    for (QGraphicsItem *item : allItems) {
        if (item->data(0).toString() == "facility") {
            QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
            if (ellipseItem) {
                QString facilityId = item->data(1).toString();
                if (!facilityId.isEmpty()) {
                    facilityItems[facilityId] = ellipseItem;
                }
            }
        }
    }
    
    qDebug() << "[AnnotationRenderer] Found" << facilityItems.size() << "facility items in scene to label";
    
    if (facilityItems.isEmpty()) {
        qDebug() << "[AnnotationRenderer] No facility items found in scene, skipping labels";
        return;
    }
    
    // 从数据库加载设施数据以获取名称
    QVector<Facility> facilities = m_facilityDao->findAll(1000);
    
    // 创建设施ID到Facility对象的映射
    QHash<QString, Facility> facilityMap;
    for (const Facility &f : facilities) {
        facilityMap[f.facilityId()] = f;
    }
    
    int rendered = 0;
    int skippedNoName = 0;
    
    // 遍历场景中的设施项，为每个项创建标注
    for (auto it = facilityItems.constBegin(); it != facilityItems.constEnd(); ++it) {
        QString facilityId = it.key();
        QGraphicsEllipseItem *ellipseItem = it.value();
        
        // 获取设施数据
        Facility facility = facilityMap.value(facilityId);
        
        // 检查设施是否有名称或编号
        QString labelText = facility.isValid() && !facility.facilityName().isEmpty() ? 
                           facility.facilityName() : 
                           facilityId;
        if (labelText.isEmpty()) {
            skippedNoName++;
            continue;
        }
        
        // 获取设施的场景坐标（使用椭圆的中心）
        QRectF ellipseRect = ellipseItem->rect();
        QPointF scenePos = ellipseRect.center() + ellipseItem->pos();
        
        qDebug() << "[AnnotationRenderer] Facility" << labelText << "scenePos:" << scenePos;
        
        // 创建标注
        QGraphicsTextItem *label = nullptr;
        if (facility.isValid()) {
            label = createFacilityLabel(facility, scenePos);
        } else {
            // 如果没有找到设施数据，创建一个简单的标注
            label = new QGraphicsTextItem(labelText);
            label->setFont(m_labelFont);
            label->setDefaultTextColor(m_labelColor);
            QRectF textRect = label->boundingRect();
            QPointF labelPos = calculateLabelPosition(scenePos, textRect.size());
            label->setPos(labelPos);
            label->setData(0, "annotation");
            label->setData(1, "facility");
            label->setData(2, facilityId);
            m_scene->addItem(label);
        }
        
        if (label) {
            m_facilityLabels.append(label);
            rendered++;
            qDebug() << "[AnnotationRenderer] ✓ Created label for facility:" << labelText;
        }
    }
    
    qDebug() << "[AnnotationRenderer] Facility labels summary: total=" << facilityItems.size()
             << "rendered=" << rendered
             << "skipped(no name)=" << skippedNoName;
    
    emit renderProgress(facilityItems.size(), facilityItems.size());
    emit renderComplete(rendered);
    
    LOG_INFO(QString("Rendered %1 facility labels").arg(rendered));
}

void AnnotationRenderer::clearAll()
{
    clearPipelineAnnotations();
    clearFacilityAnnotations();
}

void AnnotationRenderer::clearPipelineAnnotations()
{
    if (!m_scene) {
        return;
    }
    
    for (QGraphicsTextItem *label : m_pipelineLabels) {
        if (label && m_scene) {
            m_scene->removeItem(label);
        }
        delete label;
    }
    m_pipelineLabels.clear();
}

void AnnotationRenderer::clearFacilityAnnotations()
{
    if (!m_scene) {
        return;
    }
    
    for (QGraphicsTextItem *label : m_facilityLabels) {
        if (label && m_scene) {
            m_scene->removeItem(label);
        }
        delete label;
    }
    m_facilityLabels.clear();
}

void AnnotationRenderer::setPipelineLabelsVisible(bool visible)
{
    m_pipelineLabelsVisible = visible;
    if (!visible) {
        clearPipelineAnnotations();
    }
}

void AnnotationRenderer::setFacilityLabelsVisible(bool visible)
{
    m_facilityLabelsVisible = visible;
    if (!visible) {
        clearFacilityAnnotations();
    }
}

void AnnotationRenderer::setLabelFont(const QFont &font)
{
    m_labelFont = font;
}

void AnnotationRenderer::setLabelColor(const QColor &color)
{
    m_labelColor = color;
}

void AnnotationRenderer::setLabelBackgroundColor(const QColor &color)
{
    m_labelBackgroundColor = color;
}

QGraphicsTextItem* AnnotationRenderer::createPipelineLabel(const Pipeline &pipeline, const QPointF &scenePos)
{
    if (!m_scene) {
        return nullptr;
    }
    
    // 创建标注文本（显示管线编号或名称）
    QString labelText = pipeline.pipelineName().isEmpty() ? 
                        pipeline.pipelineId() : 
                        pipeline.pipelineName();
    
    if (labelText.isEmpty()) {
        qDebug() << "[AnnotationRenderer] Pipeline" << pipeline.id() << "has no name or ID, skipping label";
        return nullptr;
    }
    
    qDebug() << "[AnnotationRenderer] Creating label for pipeline:" << labelText << "at scene pos:" << scenePos;
    
    // 创建文本项
    QGraphicsTextItem *textItem = new QGraphicsTextItem(labelText);
    textItem->setFont(m_labelFont);
    textItem->setDefaultTextColor(m_labelColor);
    
    // 设置位置
    QRectF textRect = textItem->boundingRect();
    QPointF labelPos = calculateLabelPosition(scenePos, textRect.size());
    textItem->setPos(labelPos);
    
    // 添加背景（可选）
    QGraphicsRectItem *bgItem = new QGraphicsRectItem(textRect);
    bgItem->setPos(labelPos);
    bgItem->setBrush(QBrush(m_labelBackgroundColor));
    bgItem->setPen(QPen(Qt::NoPen));
    bgItem->setZValue(textItem->zValue() - 0.1);
    bgItem->setOpacity(0.8);  // 半透明背景
    
    // 设置Z值（确保在管线之上）
    textItem->setZValue(200);
    bgItem->setZValue(199);
    
    // 设置数据
    textItem->setData(0, "annotation");
    textItem->setData(1, "pipeline");
    textItem->setData(2, pipeline.pipelineId());
    
    // 背景项也设置数据标记，方便查找
    bgItem->setData(0, "annotation");
    bgItem->setData(1, "pipeline");
    bgItem->setData(2, pipeline.pipelineId());
    
    // 添加到场景
    m_scene->addItem(bgItem);
    m_scene->addItem(textItem);
    
    // 将背景项作为文本项的子项，方便管理
    bgItem->setParentItem(textItem);
    
    return textItem;
}

QGraphicsTextItem* AnnotationRenderer::createFacilityLabel(const Facility &facility, const QPointF &scenePos)
{
    if (!m_scene) {
        return nullptr;
    }
    
    // 创建标注文本（显示设施名称或编号）
    QString labelText = facility.facilityName().isEmpty() ? 
                        facility.facilityId() : 
                        facility.facilityName();
    
    if (labelText.isEmpty()) {
        return nullptr;
    }
    
    // 创建文本项
    QGraphicsTextItem *textItem = new QGraphicsTextItem(labelText);
    textItem->setFont(m_labelFont);
    textItem->setDefaultTextColor(m_labelColor);
    
    // 设置位置（设施标注在设施点上方）
    QRectF textRect = textItem->boundingRect();
    QPointF labelPos = scenePos + QPointF(-textRect.width() / 2, -textRect.height() - 10);
    textItem->setPos(labelPos);
    
    // 添加背景（可选）
    QGraphicsRectItem *bgItem = new QGraphicsRectItem(textRect);
    bgItem->setPos(labelPos);
    bgItem->setBrush(QBrush(m_labelBackgroundColor));
    bgItem->setPen(QPen(Qt::NoPen));
    bgItem->setZValue(textItem->zValue() - 0.1);
    bgItem->setOpacity(0.8);  // 半透明背景
    
    // 设置Z值（确保在设施之上）
    textItem->setZValue(250);
    bgItem->setZValue(249);
    
    // 设置数据
    textItem->setData(0, "annotation");
    textItem->setData(1, "facility");
    textItem->setData(2, facility.facilityId());
    
    // 背景项也设置数据标记，方便查找
    bgItem->setData(0, "annotation");
    bgItem->setData(1, "facility");
    bgItem->setData(2, facility.facilityId());
    
    // 添加到场景
    m_scene->addItem(bgItem);
    m_scene->addItem(textItem);
    
    // 将背景项作为文本项的子项，方便管理
    bgItem->setParentItem(textItem);
    
    return textItem;
}

QPointF AnnotationRenderer::geoToScene(const QPointF &geoPoint) const
{
    if (!m_tileMapManager) {
        qDebug() << "[AnnotationRenderer] ⚠️  TileMapManager is null, cannot convert coordinates";
        return geoPoint;  // 如果没有瓦片管理器，直接返回
    }
    
    // 检查坐标是否是有效的经纬度
    // 经纬度范围：lon: -180 到 180, lat: -90 到 90
    double lon = geoPoint.x();
    double lat = geoPoint.y();
    
    // 如果坐标看起来不像是经纬度（可能是Web Mercator或其他投影坐标）
    if (qAbs(lon) > 180 || qAbs(lat) > 90) {
        qDebug() << "[AnnotationRenderer] ⚠️  Coordinates" << geoPoint << "don't look like lat/lon, may be in different CRS";
        // 如果坐标不在经纬度范围内，可能是投影坐标
        // 这种情况下，我们需要找到场景中已有的管线项，使用它们的坐标转换
        // 暂时返回一个无效坐标，让标注不显示（避免显示在错误位置）
        return QPointF(qQNaN(), qQNaN());
    }
    
    // 使用瓦片管理器进行坐标转换
    QPointF scenePos = m_tileMapManager->geoToScene(lon, lat);
    qDebug() << "[AnnotationRenderer] Geo" << geoPoint << "-> Scene" << scenePos;
    return scenePos;
}

QPointF AnnotationRenderer::calculateLabelPosition(const QPointF &basePos, const QSizeF &labelSize) const
{
    // 简单的位置计算：标注在点的右上方
    // 可以后续优化为智能避让算法
    return basePos + QPointF(5, -labelSize.height() - 5);
}

bool AnnotationRenderer::isInBounds(const QPointF &pos, const QRectF &bounds) const
{
    if (bounds.isNull()) {
        qDebug() << "[AnnotationRenderer] Bounds is null, allowing all positions";
        return true;  // 如果bounds为空，允许所有位置
    }
    bool inBounds = bounds.contains(pos);
    if (!inBounds) {
        qDebug() << "[AnnotationRenderer] Position" << pos << "is out of bounds" << bounds;
    }
    return inBounds;
}

