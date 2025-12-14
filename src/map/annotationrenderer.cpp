#include "map/annotationrenderer.h"
#include "tilemap/tilemapmanager.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/common/logger.h"
#include "core/common/entitystate.h"
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
    // 使用图形项指针作为key，这样可以处理pipelineId为空的情况（新绘制的管线可能还没有ID）
    QHash<QGraphicsItem*, QGraphicsPathItem*> pipelineItemsByItem;  // item -> pathItem
    QHash<QString, QGraphicsPathItem*> pipelineItemsById;  // pipelineId -> pathItem (用于数据库查询)
    
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
                
                // 无论pipelineId是否为空，都添加到列表中（新绘制的管线可能还没有ID）
                pipelineItemsByItem[item] = pathItem;
                if (!pipelineId.isEmpty()) {
                    pipelineItemsById[pipelineId] = pathItem;
                }
            }
        }
    }
    
    qDebug() << "[AnnotationRenderer] Found" << pipelineItemsByItem.size() << "pipeline items in scene to label";
    
    if (pipelineItemsByItem.isEmpty()) {
        qDebug() << "[AnnotationRenderer] No pipeline items found in scene, skipping labels";
        return;
    }
    
    // 从数据库加载管线数据以获取名称（只加载有ID的管线）
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
    for (auto it = pipelineItemsByItem.constBegin(); it != pipelineItemsByItem.constEnd(); ++it) {
        QGraphicsItem *item = it.key();
        QGraphicsPathItem *pathItem = it.value();
        QString pipelineId = item->data(1).toString();
        
        // 检查图形项是否还在场景中（可能被撤销操作移除了）
        if (!item || !item->scene() || item->scene() != m_scene) {
            qDebug() << "[AnnotationRenderer] Pipeline item not in scene, skipping";
            continue;
        }
        
        // 优先从图形项的data(3)获取管线名称（新绘制的管线可能还没有保存到数据库）
        QString pipelineName = item->data(3).toString();
        
        // 获取管线数据（优先从数据库，如果pipelineId为空则跳过数据库查询）
        Pipeline pipeline;
        if (!pipelineId.isEmpty()) {
            pipeline = pipelineMap.value(pipelineId);
        }
        
        // 对于新绘制的管线，优先使用图形项中的名称（因为可能还没有保存到数据库）
        // 只有当图形项中没有名称时，才使用数据库中的名称
        if (pipelineName.isEmpty() && pipeline.isValid() && !pipeline.pipelineName().isEmpty()) {
            pipelineName = pipeline.pipelineName();
        }
        
        // 调试信息
        qDebug() << "[AnnotationRenderer] Pipeline ID:" << pipelineId 
                 << "Name from item data(3):" << item->data(3).toString()
                 << "Name from database:" << (pipeline.isValid() ? pipeline.pipelineName() : QString("N/A"))
                 << "Final name:" << pipelineName;
        
        // 确定标注文本：优先使用名称，如果没有名称则使用编号，如果都没有则使用类型
        QString labelText;
        if (!pipelineName.isEmpty()) {
            labelText = pipelineName;
        } else if (!pipelineId.isEmpty()) {
            labelText = pipelineId;
        } else {
            // 如果既没有名称也没有ID，尝试使用类型名称
            QString itemPipelineType = item->data(2).toString();
            if (!itemPipelineType.isEmpty()) {
                // 使用类型名称作为临时标注
                if (itemPipelineType == "water_supply") labelText = "给水";
                else if (itemPipelineType == "sewage") labelText = "排水";
                else if (itemPipelineType == "gas") labelText = "燃气";
                else if (itemPipelineType == "electric") labelText = "电力";
                else if (itemPipelineType == "telecom") labelText = "通信";
                else if (itemPipelineType == "heat") labelText = "供热";
                else labelText = itemPipelineType;
            } else {
                skippedNoName++;
                qDebug() << "[AnnotationRenderer] Skipping pipeline with no name, id, or type";
                continue;
            }
        }
        
        // 获取管线的场景坐标（使用路径的中点）
        QPainterPath path = pathItem->path();
        QRectF pathBounds = path.boundingRect();
        QPointF scenePos = pathBounds.center() + pathItem->pos();
        
        qDebug() << "[AnnotationRenderer] Pipeline" << labelText << "scenePos:" << scenePos;
        
        // 创建标注
        // 注意：对于新绘制的管线，即使pipeline.isValid()为true，也要使用图形项中的名称
        // 因为图形项中的名称是最新的（可能用户刚刚修改过）
        QGraphicsTextItem *label = nullptr;
        
        // 如果图形项中有名称，或者数据库中没有数据，使用我们确定的labelText
        // 只有当图形项中没有名称且数据库中有数据时，才使用createPipelineLabel
        if (pipeline.isValid() && pipelineName.isEmpty() && !pipeline.pipelineName().isEmpty()) {
            // 图形项中没有名称，但数据库中有，使用数据库中的名称
            label = createPipelineLabel(pipeline, scenePos);
        } else {
            // 使用我们确定的labelText（优先使用图形项中的名称）
            label = new QGraphicsTextItem(labelText);
            label->setFont(m_labelFont);
            label->setDefaultTextColor(m_labelColor);
            QRectF textRect = label->boundingRect();
            QPointF labelPos = calculateLabelPosition(scenePos, textRect.size());
            label->setPos(labelPos);
            label->setData(0, "annotation");
            label->setData(1, "pipeline");
            label->setData(2, pipelineId);
            
            // 添加背景（可选）
            QGraphicsRectItem *bgItem = new QGraphicsRectItem(textRect);
            bgItem->setPos(labelPos);
            bgItem->setBrush(QBrush(m_labelBackgroundColor));
            bgItem->setPen(QPen(Qt::NoPen));
            bgItem->setZValue(label->zValue() - 0.1);
            bgItem->setOpacity(0.8);  // 半透明背景
            
            // 设置Z值（确保在管线之上）
            label->setZValue(200);
            bgItem->setZValue(199);
            
            // 背景项也设置数据标记，方便查找
            bgItem->setData(0, "annotation");
            bgItem->setData(1, "pipeline");
            bgItem->setData(2, pipelineId);
            
            // 让标注不拦截鼠标事件，这样点击会穿透到下面的管线
            label->setAcceptHoverEvents(false);
            label->setFlag(QGraphicsItem::ItemIsSelectable, false);
            label->setFlag(QGraphicsItem::ItemIsFocusable, false);
            bgItem->setAcceptHoverEvents(false);
            bgItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
            bgItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
            
            // 添加到场景
            m_scene->addItem(bgItem);
            m_scene->addItem(label);
            
            // 将背景项作为文本项的子项，方便管理
            bgItem->setParentItem(label);
        }
        
        if (label) {
            m_pipelineLabels.append(label);
            rendered++;
            qDebug() << "[AnnotationRenderer] ✓ Created label for pipeline:" << labelText;
        }
    }
    
    qDebug() << "[AnnotationRenderer] Pipeline labels summary: total=" << pipelineItemsByItem.size()
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
    // 使用图形项指针作为key，这样可以处理facilityId为空的情况
    QHash<QGraphicsItem*, QGraphicsEllipseItem*> facilityItemsMap;  // item -> ellipseItem
    QHash<QString, QGraphicsEllipseItem*> facilityItemsById;  // facilityId -> item (用于数据库查询)
    
    for (QGraphicsItem *item : allItems) {
        if (item->data(0).toString() == "facility") {
            QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
            if (ellipseItem) {
                facilityItemsMap[item] = ellipseItem;
                QString facilityId = item->data(1).toString();
                if (!facilityId.isEmpty()) {
                    facilityItemsById[facilityId] = ellipseItem;
                }
            }
        }
    }
    
    qDebug() << "[AnnotationRenderer] Found" << facilityItemsMap.size() << "facility items in scene to label";
    
    if (facilityItemsMap.isEmpty()) {
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
    for (auto it = facilityItemsMap.constBegin(); it != facilityItemsMap.constEnd(); ++it) {
        QGraphicsItem *item = it.key();
        QGraphicsEllipseItem *ellipseItem = it.value();
        
        // 检查图形项是否还在场景中（可能被撤销操作移除了）
        if (!item || !item->scene() || item->scene() != m_scene) {
            qDebug() << "[AnnotationRenderer] Facility item not in scene, skipping";
            continue;
        }
        
        QString facilityId = item->data(1).toString();
        
        // 检查实体状态（用于区分新添加的设施和已删除的设施）
        QVariant stateVariant = item->data(100);
        EntityState entityState = EntityState::Detached;
        if (stateVariant.isValid()) {
            entityState = static_cast<EntityState>(stateVariant.toInt());
        }
        
        // 如果实体状态是Deleted，跳过（已标记为删除）
        if (entityState == EntityState::Deleted) {
            qDebug() << "[AnnotationRenderer] Facility marked as deleted, skipping:" << facilityId;
            skippedNoName++;
            continue;
        }
        
        // 如果facilityId为空，跳过（无法从数据库查询）
        if (facilityId.isEmpty()) {
            // 可能是新绘制的设施，还没保存到数据库，使用图形项中的名称
            QString facilityName = item->data(3).toString();
            if (facilityName.isEmpty()) {
                skippedNoName++;
                qDebug() << "[AnnotationRenderer] Skipping facility with no id/name, item:" << item;
                continue;
            }
        }
        
        // 获取设施数据（从数据库查询）
        Facility facility = facilityMap.value(facilityId);
        
        // 如果设施不在数据库中，需要判断是新添加的还是已删除的
        if (!facility.isValid() && !facilityId.isEmpty()) {
            // 如果实体状态是Added，说明是新添加的，还没保存到数据库，应该显示标注
            if (entityState == EntityState::Added) {
                qDebug() << "[AnnotationRenderer] Facility is newly added (not yet saved), will show label:" << facilityId;
                // 继续处理，使用图形项中的名称
            } else if (!facilityId.startsWith("TEMP_")) {
                // 如果状态不是Added，且不是临时ID，说明可能已被删除，跳过
                qDebug() << "[AnnotationRenderer] Facility not found in database (deleted), skipping:" << facilityId;
                skippedNoName++;
                continue;
            }
        }
        
        // 优先从图形项的data(3)获取设施名称（新绘制的设施可能还没有保存到数据库）
        QString facilityName = item->data(3).toString();
        
        // 如果数据库中有设施数据，优先使用数据库中的名称
        if (facility.isValid() && !facility.facilityName().isEmpty()) {
            facilityName = facility.facilityName();
        }
        
        // 确定标注文本：优先使用名称，如果没有名称则使用编号
        QString labelText;
        if (!facilityName.isEmpty()) {
            labelText = facilityName;
        } else if (!facilityId.isEmpty()) {
            labelText = facilityId;
        } else {
            // facilityId为空，尝试从tooltip获取
            QString tooltip = item->toolTip();
            if (!tooltip.isEmpty()) {
                // tooltip格式通常是 "名称\n类型: xxx"
                QStringList lines = tooltip.split('\n');
                if (!lines.isEmpty()) {
                    labelText = lines.first().trimmed();
                }
            }
            // 如果tooltip也没有，使用设施类型
            if (labelText.isEmpty()) {
                QString facilityType = item->data(2).toString();
                if (!facilityType.isEmpty()) {
                    // 使用类型名称作为标注
                    if (facilityType == "valve") labelText = "阀门";
                    else if (facilityType == "manhole") labelText = "井盖";
                    else if (facilityType == "pump_station") labelText = "泵站";
                    else if (facilityType == "transformer") labelText = "变压器";
                    else if (facilityType == "regulator" || facilityType == "pressure_station") labelText = "调压站";
                    else if (facilityType == "junction_box") labelText = "接线盒";
                    else labelText = facilityType;
                }
            }
        }
        
        if (labelText.isEmpty()) {
            skippedNoName++;
            qDebug() << "[AnnotationRenderer] Skipping facility with no name/id, item:" << item;
            continue;
        }
        
        // 获取设施的场景坐标（使用椭圆的中心）
        QRectF ellipseRect = ellipseItem->rect();
        QPointF scenePos = ellipseRect.center() + ellipseItem->pos();
        
        qDebug() << "[AnnotationRenderer] Facility" << labelText << "facilityId:" << facilityId << "scenePos:" << scenePos;
        
        // 创建标注
        QGraphicsTextItem *label = nullptr;
        if (facility.isValid()) {
            label = createFacilityLabel(facility, scenePos);
        } else {
            // 如果没有找到设施数据（可能是新绘制的，还没保存），创建一个简单的标注
            label = new QGraphicsTextItem(labelText);
            label->setFont(m_labelFont);
            label->setDefaultTextColor(m_labelColor);
            QRectF textRect = label->boundingRect();
            QPointF labelPos = calculateLabelPosition(scenePos, textRect.size());
            label->setPos(labelPos);
            label->setData(0, "annotation");
            label->setData(1, "facility");
            label->setData(2, facilityId.isEmpty() ? QString("pending_%1").arg(reinterpret_cast<quintptr>(item)) : facilityId);
            // 存储关联的图形项指针，用于后续查找
            label->setData(10, reinterpret_cast<quintptr>(item));
            
            // 让标注不拦截鼠标事件，这样点击会穿透到下面的设施
            label->setAcceptHoverEvents(false);
            label->setFlag(QGraphicsItem::ItemIsSelectable, false);
            label->setFlag(QGraphicsItem::ItemIsFocusable, false);
            
            // 设置Z值（确保在设施之上，但不会拦截鼠标事件）
            label->setZValue(250);
            
            m_scene->addItem(label);
        }
        
        if (label) {
            m_facilityLabels.append(label);
            rendered++;
            qDebug() << "[AnnotationRenderer] ✓ Created label for facility:" << labelText;
        }
    }
    
    qDebug() << "[AnnotationRenderer] Facility labels summary: total=" << facilityItemsMap.size()
             << "rendered=" << rendered
             << "skipped(no name)=" << skippedNoName;
    
    emit renderProgress(facilityItemsMap.size(), facilityItemsMap.size());
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
    
    // 让标注不拦截鼠标事件，这样点击会穿透到下面的管线
    textItem->setAcceptHoverEvents(false);
    textItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    bgItem->setAcceptHoverEvents(false);
    bgItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    bgItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    
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
    
    // 让标注不拦截鼠标事件，这样点击会穿透到下面的设施
    textItem->setAcceptHoverEvents(false);
    textItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    bgItem->setAcceptHoverEvents(false);
    bgItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    bgItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    
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

