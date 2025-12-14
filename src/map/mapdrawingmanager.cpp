#include "map/mapdrawingmanager.h"
#include "dao/facilitydao.h"
#include <QPainterPath>
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <QPainter>
#include <QPixmap>
#include <limits>

MapDrawingManager::MapDrawingManager(QGraphicsScene *scene,
                                   QGraphicsView *view,
                                   TileMapManager *tileManager,
                                   QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_view(view)
    , m_tileManager(tileManager)
    , m_mode(NoDrawing)
    , m_previewLine(nullptr)
    , m_previewPoint(nullptr)
    , m_facilitySnapIndicator(nullptr)
    , m_drawingColor(QColor("#1890ff"))  // 默认蓝色
    , m_lineWidth(3)                      // 默认3px
{
    qDebug() << "MapDrawingManager initialized";
}

MapDrawingManager::~MapDrawingManager()
{
    clearTemporaryGraphics();
}

void MapDrawingManager::startDrawingPipeline(const QString &pipelineType)
{
    qDebug() << "Start drawing pipeline:" << pipelineType;
    
    // 清除之前的绘制状态
    cancelDrawing();
    
    m_mode = DrawingPolyline;
    m_currentType = pipelineType;
    m_points.clear();
    
    // 更新光标样式
    updateCursor();
    
    emit drawingStateChanged(true);
}

void MapDrawingManager::startDrawingFacility(const QString &facilityType)
{
    qDebug() << "Start drawing facility:" << facilityType;
    
    // 清除之前的绘制状态
    cancelDrawing();
    
    m_mode = DrawingPoint;
    m_currentType = facilityType;
    m_points.clear();
    
    // 更新光标样式
    updateCursor();
    
    emit drawingStateChanged(true);
}

void MapDrawingManager::setDrawingStyle(const QColor &color, int lineWidth)
{
    m_drawingColor = color;
    m_lineWidth = qBound(1, lineWidth, 10);  // 限制化1-10px
    
    qDebug() << "Drawing style updated: color=" << m_drawingColor.name() 
             << "lineWidth=" << m_lineWidth;
    
    // 如果正在绘制，更新预览样式
    if (m_previewLine) {
        QPen pen = m_previewLine->pen();
        pen.setColor(m_drawingColor);
        pen.setWidth(m_lineWidth);
        m_previewLine->setPen(pen);
    }
    
    // 更新已绘制点的颜色
    for (auto marker : m_pointMarkers) {
        marker->setBrush(QBrush(m_drawingColor));
        QPen pen = marker->pen();
        pen.setColor(m_drawingColor.darker(120));
        marker->setPen(pen);
    }
}

void MapDrawingManager::cancelDrawing()
{
    if (m_mode == NoDrawing) {
        return;
    }
    
    qDebug() << "Cancel drawing";
    
    clearTemporaryGraphics();
    m_points.clear();
    m_connectedFacilityIds.clear();
    m_mode = NoDrawing;
    m_currentType.clear();
    
    // 恢复默认光标
    updateCursor();
    
    emit drawingStateChanged(false);
}

void MapDrawingManager::finishDrawing()
{
    if (m_mode == NoDrawing) {
        return;
    }
    
    qDebug() << "Finish drawing, mode:" << m_mode << "points:" << m_points.size();
    
    if (m_mode == DrawingPolyline) {
        // 管线至少需要2个点
        if (m_points.size() >= 2) {
            QString wkt = generateLineStringWKT(m_points);
            emit pipelineDrawingFinished(m_currentType, wkt, m_points, m_connectedFacilityIds);
            qDebug() << "Pipeline drawing finished, WKT:" << wkt;
            qDebug() << "Connected facilities:" << m_connectedFacilityIds;
            
            // 清除当前绘制的点和临时图形，但保持绘制状态
            clearTemporaryGraphics();
            m_points.clear();
            m_connectedFacilityIds.clear();
            
            qDebug() << "✨ Ready to draw next pipeline of type:" << m_currentType;
        } else {
            qDebug() << "Pipeline需要至少2个点，当前只有" << m_points.size() << "个点";
        }
    } else if (m_mode == DrawingPoint) {
        // 设施只需要1个点
        if (m_points.size() >= 1) {
            QPointF point = m_points.first();
            QString wkt = generatePointWKT(point);
            emit facilityDrawingFinished(m_currentType, wkt, point);
            qDebug() << "Facility drawing finished, WKT:" << wkt;
            
            // 清除当前绘制的点和临时图形，但保持绘制状态
            clearTemporaryGraphics();
            m_points.clear();
            
            qDebug() << "✨ Ready to draw next facility of type:" << m_currentType;
        }
    }
    
    // 注意：不再调用 cancelDrawing()，保持绘制状态
    // 用户可以继续绘制下一条同类型的管线
}

void MapDrawingManager::handleMouseClick(const QPointF &scenePos)
{
    if (m_mode == NoDrawing) {
        return;
    }
    
    qDebug() << "Mouse click at scene pos:" << scenePos;
    
    if (m_mode == DrawingPolyline) {
        // 检测附近是否有设施，如果有则吸附到设施位置
        QPointF actualPos = scenePos;
        QString connectedFacilityId;
        
        NearbyFacility nearby = findNearbyFacility(scenePos, 20.0);
        if (!nearby.facilityId.isEmpty()) {
            actualPos = nearby.scenePos;
            connectedFacilityId = nearby.facilityId;
            qDebug() << "Snapped to facility:" << connectedFacilityId << "at" << actualPos;
        }
        
        // 绘制管线：添加点到列表
        m_points.append(actualPos);
        m_connectedFacilityIds.append(connectedFacilityId);  // 记录连接的设施ID
        
        // 创建点标记
        QGraphicsEllipseItem *marker = new QGraphicsEllipseItem(
            actualPos.x() - POINT_MARKER_SIZE / 2,
            actualPos.y() - POINT_MARKER_SIZE / 2,
            POINT_MARKER_SIZE,
            POINT_MARKER_SIZE
        );
        marker->setBrush(QBrush(m_drawingColor));  // 使用自定义颜色
        marker->setPen(QPen(m_drawingColor.darker(120), 2));  // 边框颜色深一点
        marker->setZValue(1000); // 确保在最上层
        m_scene->addItem(marker);
        m_pointMarkers.append(marker);
        
        // 清除设施吸附指示器
        clearFacilitySnapIndicator();
        
        qDebug() << "Added point" << m_points.size() << "at" << actualPos;
        
    } else if (m_mode == DrawingPoint) {
        // 绘制设施：只需要一个点，直接完成
        m_points.clear();
        m_points.append(scenePos);
        
        qDebug() << "Facility point set at" << scenePos;
        
        // 立即完成绘制
        finishDrawing();
    }
}

void MapDrawingManager::handleMouseMove(const QPointF &scenePos)
{
    if (m_mode == NoDrawing) {
        return;
    }
    
    if (m_mode == DrawingPolyline && m_points.size() > 0) {
        // 检测附近是否有设施，如果有则吸附
        QPointF actualPos = scenePos;
        NearbyFacility nearby = findNearbyFacility(scenePos, 20.0);
        if (!nearby.facilityId.isEmpty()) {
            actualPos = nearby.scenePos;
            updateFacilitySnapIndicator(actualPos);
        } else {
            clearFacilitySnapIndicator();
        }
        
        // 更新预览线（橡皮筋效果）
        updatePreviewLine(actualPos);
    } else if (m_mode == DrawingPoint) {
        // 更新预览点
        createPreviewPoint(scenePos);
    }
}

void MapDrawingManager::handleRightClick(const QPointF &scenePos)
{
    Q_UNUSED(scenePos);
    
    if (m_mode == NoDrawing) {
        return;
    }
    
    if (m_mode == DrawingPolyline) {
        if (m_points.size() >= 2) {
            // 有足够的点，完成当前管线绘制（但保持绘制状态）
            qDebug() << "Right click - finish current pipeline";
            finishDrawing();
        } else if (m_points.size() == 1) {
            // 只有一个点，清除并继续
            qDebug() << "Right click - clear single point, continue drawing";
            clearTemporaryGraphics();
            m_points.clear();
        } else {
            // 没有点，右键空白处 → 取消绘制状态
            qDebug() << "Right click on empty area - cancel drawing mode";
            cancelDrawing();
        }
    } else if (m_mode == DrawingPoint) {
        // 设施绘制模式，右键取消
        qDebug() << "Right click - cancel facility drawing";
        cancelDrawing();
    }
}

void MapDrawingManager::handleDoubleClick(const QPointF &scenePos)
{
    if (m_mode != DrawingPolyline) {
        return;
    }

    // 若双击位置与最后一个点不同，则补充终点
    if (m_points.isEmpty() || m_points.last() != scenePos) {
        handleMouseClick(scenePos);
    }

    // 结束当前这条管线（保持绘制状态，便于继续下一条）
    qDebug() << "Double click - finish current pipeline";
    finishDrawing();
}

void MapDrawingManager::clearTemporaryGraphics()
{
    // 清除点标记
    for (QGraphicsEllipseItem *marker : m_pointMarkers) {
        m_scene->removeItem(marker);
        delete marker;
    }
    m_pointMarkers.clear();
    
    // 清除预览线
    if (m_previewLine) {
        m_scene->removeItem(m_previewLine);
        delete m_previewLine;
        m_previewLine = nullptr;
    }
    
    // 清除预览点
    if (m_previewPoint) {
        m_scene->removeItem(m_previewPoint);
        delete m_previewPoint;
        m_previewPoint = nullptr;
    }
    
    // 清除设施吸附指示器
    clearFacilitySnapIndicator();
}

void MapDrawingManager::updatePreviewLine(const QPointF &currentPos)
{
    if (m_points.isEmpty()) {
        return;
    }
    
    // 移除旧的预览线
    if (m_previewLine) {
        m_scene->removeItem(m_previewLine);
        delete m_previewLine;
    }
    
    // 创建新的预览路径
    QPainterPath path;
    path.moveTo(m_points.first());
    
    for (int i = 1; i < m_points.size(); ++i) {
        path.lineTo(m_points[i]);
    }
    
    // 添加到当前鼠标位置的预览线
    path.lineTo(currentPos);
    
    // 创建预览线图形项
    m_previewLine = new QGraphicsPathItem(path);
    m_previewLine->setPen(QPen(m_drawingColor, m_lineWidth, Qt::DashLine));  // 使用自定义样式
    m_previewLine->setZValue(999); // 确保在点标记下方
    m_scene->addItem(m_previewLine);
}

void MapDrawingManager::createPreviewPoint(const QPointF &pos)
{
    // 移除旧的预览点
    if (m_previewPoint) {
        m_scene->removeItem(m_previewPoint);
        delete m_previewPoint;
    }
    
    // 创建新的预览点
    m_previewPoint = new QGraphicsEllipseItem(
        pos.x() - PREVIEW_POINT_SIZE / 2,
        pos.y() - PREVIEW_POINT_SIZE / 2,
        PREVIEW_POINT_SIZE,
        PREVIEW_POINT_SIZE
    );
    QColor previewColor = m_drawingColor;
    previewColor.setAlpha(150);  // 半透明
    m_previewPoint->setBrush(QBrush(previewColor));  // 使用自定义颜色
    m_previewPoint->setPen(QPen(m_drawingColor, 2));
    m_previewPoint->setZValue(1000);
    m_scene->addItem(m_previewPoint);
}

QString MapDrawingManager::generateLineStringWKT(const QVector<QPointF> &points)
{
    if (points.size() < 2) {
        return QString();
    }
    
    // 将场景坐标转换为地理坐标，生成 LINESTRING WKT
    QStringList coords;
    for (const QPointF &scenePoint : points) {
        QPointF geoPoint = sceneToGeo(scenePoint);
        coords << QString("%1 %2").arg(geoPoint.x(), 0, 'f', 8).arg(geoPoint.y(), 0, 'f', 8);
    }
    
    return QString("LINESTRING(%1)").arg(coords.join(", "));
}

QString MapDrawingManager::generatePointWKT(const QPointF &point)
{
    // 将场景坐标转换为地理坐标，生成 POINT WKT
    QPointF geoPoint = sceneToGeo(point);
    return QString("POINT(%1 %2)")
        .arg(geoPoint.x(), 0, 'f', 8)
        .arg(geoPoint.y(), 0, 'f', 8);
}

QPointF MapDrawingManager::sceneToGeo(const QPointF &scenePos)
{
    if (!m_tileManager) {
        // 如果没有瓦片管理器，返回原始坐标
        return scenePos;
    }
    
    // 使用 TileMapManager 的坐标转换方法
    return m_tileManager->sceneToGeo(scenePos);
}

QPointF MapDrawingManager::geoToScene(const QPointF &geoPos)
{
    if (!m_tileManager) {
        return geoPos;
    }
    
    // 使用 TileMapManager 的坐标转换方法
    return m_tileManager->geoToScene(geoPos.x(), geoPos.y());
}

MapDrawingManager::NearbyFacility MapDrawingManager::findNearbyFacility(const QPointF &scenePos, double tolerancePixels)
{
    NearbyFacility result;
    result.facilityId = QString();
    result.distance = std::numeric_limits<double>::max();
    
    if (!m_tileManager) {
        return result;
    }
    
    // 将场景坐标转换为地理坐标
    QPointF geoPos = sceneToGeo(scenePos);
    
    // 将像素容差转换为米（近似）
    // 在缩放级别z下，1像素 ≈ 156543.03392 * cos(lat) / 2^z 米
    int zoom = m_tileManager->getZoom();
    double metersPerPixel = 156543.03392 * cos(qDegreesToRadians(geoPos.y())) / qPow(2.0, zoom);
    double toleranceMeters = tolerancePixels * metersPerPixel;
    
    // 查找附近的设施
    FacilityDAO facilityDao;
    QVector<Facility> facilities = facilityDao.findNearPoint(geoPos.x(), geoPos.y(), toleranceMeters, 10);
    
    if (facilities.isEmpty()) {
        return result;
    }
    
    // 找到最近的设施
    for (const Facility &facility : facilities) {
        QPointF facilityGeo = facility.coordinate();
        QPointF facilityScene = geoToScene(facilityGeo);
        
        double distance = QLineF(scenePos, facilityScene).length();
        if (distance < result.distance && distance <= tolerancePixels) {
            result.facilityId = facility.facilityId();
            result.scenePos = facilityScene;
            result.distance = distance;
        }
    }
    
    return result;
}

void MapDrawingManager::updateFacilitySnapIndicator(const QPointF &facilityPos)
{
    // 清除旧的指示器
    clearFacilitySnapIndicator();
    
    // 创建新的指示器（一个高亮的圆圈）
    m_facilitySnapIndicator = new QGraphicsEllipseItem(
        facilityPos.x() - 10,
        facilityPos.y() - 10,
        20,
        20
    );
    m_facilitySnapIndicator->setBrush(QBrush(Qt::NoBrush));
    m_facilitySnapIndicator->setPen(QPen(QColor(255, 255, 0), 3, Qt::DashLine));  // 黄色虚线
    m_facilitySnapIndicator->setZValue(1001);  // 确保在最上层
    m_scene->addItem(m_facilitySnapIndicator);
}

void MapDrawingManager::clearFacilitySnapIndicator()
{
    if (m_facilitySnapIndicator) {
        m_scene->removeItem(m_facilitySnapIndicator);
        delete m_facilitySnapIndicator;
        m_facilitySnapIndicator = nullptr;
    }
}

QCursor MapDrawingManager::createPipelineCursor(const QString &pipelineType)
{
    // 根据管线类型创建对应颜色的十字光标
    QColor color;
    QString text;
    
    if (pipelineType == "water_supply") {
        color = QColor(0, 112, 192);    // 蓝色 - 给水
        text = "给水";
    } else if (pipelineType == "sewage") {
        color = QColor(112, 48, 160);   // 紫色 - 排水
        text = "排水";
    } else if (pipelineType == "gas") {
        color = QColor(255, 192, 0);    // 黄色 - 燃气
        text = "燃气";
    } else if (pipelineType == "electric") {
        color = QColor(255, 0, 0);      // 红色 - 电力
        text = "电力";
    } else if (pipelineType == "telecom") {
        color = QColor(0, 176, 80);     // 绿色 - 通信
        text = "通信";
    } else if (pipelineType == "heat") {
        color = QColor(255, 128, 0);    // 橙色 - 供热
        text = "供热";
    } else {
        color = QColor(128, 128, 128);  // 灰色 - 默认
        text = "?";
    }
    
    // 创建 32x32 的光标图像
    const int size = 32;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制十字准星
    QPen pen(color, 3, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);
    
    // 水平线
    painter.drawLine(4, size/2, size-4, size/2);
    // 垂直线
    painter.drawLine(size/2, 4, size/2, size-4);
    
    // 绘制中心圆点（填充）
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(size/2, size/2), 4, 4);
    
    // 绘制白色边框（增强可见性）
    painter.setPen(QPen(Qt::white, 5, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(4, size/2, size-4, size/2);
    painter.drawLine(size/2, 4, size/2, size-4);
    
    // 重绘颜色线（在白色上方）
    painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(4, size/2, size-4, size/2);
    painter.drawLine(size/2, 4, size/2, size-4);
    
    // 绘制中心圆点
    painter.setBrush(QBrush(color));
    painter.setPen(QPen(Qt::white, 1));
    painter.drawEllipse(QPointF(size/2, size/2), 4, 4);
    
    painter.end();
    
    // 创建光标，热点在中心
    return QCursor(pixmap, size/2, size/2);
}

void MapDrawingManager::updateCursor()
{
    if (!m_view) {
        return;
    }
    
    if (m_mode == DrawingPolyline) {
        // 绘制管线模式：设置对应类型的光标
        QCursor cursor = createPipelineCursor(m_currentType);
        m_view->setCursor(cursor);
        qDebug() << "✨ Set pipeline cursor for type:" << m_currentType;
    } else if (m_mode == DrawingPoint) {
        // 绘制设施模式：使用十字光标
        m_view->setCursor(Qt::CrossCursor);
    } else {
        // 非绘制模式：恢复默认光标
        m_view->setCursor(Qt::ArrowCursor);
    }
}
