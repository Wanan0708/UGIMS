#include "core/io/drawingdatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QDebug>

DrawingDataManager::DrawingDataManager()
{
}

DrawingDataManager::~DrawingDataManager()
{
}

bool DrawingDataManager::saveToFile(const QString &filePath, 
                                   QGraphicsScene *scene,
                                   const QHash<QGraphicsItem*, Pipeline> &pipelineHash)
{
    if (!scene) {
        qDebug() << "❌ Scene is null";
        return false;
    }
    
    QJsonObject root;
    root["version"] = "1.0";
    root["type"] = "GisDrawingData";
    
    QJsonArray entitiesArray;
    
    // 遍历场景中的所有项
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem *item : items) {
        QString entityType = item->data(0).toString();
        
        // 只保存管线和设施
        if (entityType == "pipeline" || entityType == "facility") {
            QJsonObject entityObj = serializeGraphicsItem(item);
            
            // 如果有管线数据，添加详细信息
            if (pipelineHash.contains(item)) {
                Pipeline pipeline = pipelineHash[item];
                QJsonObject pipelineObj;
                pipelineObj["id"] = pipeline.id();
                pipelineObj["pipelineName"] = pipeline.pipelineName();
                pipelineObj["pipelineId"] = pipeline.pipelineId();
                pipelineObj["pipelineType"] = pipeline.pipelineType();
                pipelineObj["diameterMm"] = pipeline.diameterMm();
                pipelineObj["material"] = pipeline.material();
                pipelineObj["lengthM"] = pipeline.lengthM();
                pipelineObj["depthM"] = pipeline.depthM();
                pipelineObj["pressureClass"] = pipeline.pressureClass();
                pipelineObj["buildDate"] = pipeline.buildDate().toString(Qt::ISODate);
                pipelineObj["builder"] = pipeline.builder();
                pipelineObj["status"] = pipeline.status();
                pipelineObj["remarks"] = pipeline.remarks();
                
                entityObj["pipelineData"] = pipelineObj;
            }
            
            entitiesArray.append(entityObj);
        }
    }
    
    root["entities"] = entitiesArray;
    root["count"] = entitiesArray.size();
    
    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "❌ Failed to open file for writing:" << filePath;
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "✅ Saved" << entitiesArray.size() << "entities to" << filePath;
    return true;
}

bool DrawingDataManager::loadFromFile(const QString &filePath,
                                     QGraphicsScene *scene,
                                     QHash<QGraphicsItem*, Pipeline> &pipelineHash,
                                     int &nextId)
{
    if (!scene) {
        qDebug() << "❌ Scene is null";
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "❌ Failed to open file for reading:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "❌ Invalid JSON format";
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 验证格式
    if (root["type"].toString() != "GisDrawingData") {
        qDebug() << "❌ Invalid file type";
        return false;
    }
    
    QJsonArray entitiesArray = root["entities"].toArray();
    int loadedCount = 0;
    int maxId = 0;
    
    for (const QJsonValue &value : entitiesArray) {
        QJsonObject entityObj = value.toObject();
        
        // 反序列化图形项
        QGraphicsItem *item = deserializeGraphicsItem(entityObj);
        if (item) {
            scene->addItem(item);
            
            // 如果有管线数据，恢复
            if (entityObj.contains("pipelineData")) {
                QJsonObject pipelineObj = entityObj["pipelineData"].toObject();
                
                Pipeline pipeline;
                pipeline.setId(pipelineObj["id"].toInt());
                pipeline.setPipelineName(pipelineObj["pipelineName"].toString());
                pipeline.setPipelineId(pipelineObj["pipelineId"].toString());
                pipeline.setPipelineType(pipelineObj["pipelineType"].toString());
                pipeline.setDiameterMm(pipelineObj["diameterMm"].toInt());
                pipeline.setMaterial(pipelineObj["material"].toString());
                pipeline.setLengthM(pipelineObj["lengthM"].toDouble());
                pipeline.setDepthM(pipelineObj["depthM"].toDouble());
                pipeline.setPressureClass(pipelineObj["pressureClass"].toString());
                pipeline.setBuildDate(QDate::fromString(pipelineObj["buildDate"].toString(), Qt::ISODate));
                pipeline.setBuilder(pipelineObj["builder"].toString());
                pipeline.setStatus(pipelineObj["status"].toString());
                pipeline.setRemarks(pipelineObj["remarks"].toString());
                
                pipelineHash[item] = pipeline;
                
                // 更新最大ID
                if (pipeline.id() > maxId) {
                    maxId = pipeline.id();
                }
            }
            
            loadedCount++;
        }
    }
    
    // 更新下一个ID
    nextId = maxId + 1;
    
    qDebug() << "✅ Loaded" << loadedCount << "entities from" << filePath;
    qDebug() << "📊 Next ID set to:" << nextId;
    
    return true;
}

QJsonObject DrawingDataManager::serializeGraphicsItem(QGraphicsItem *item)
{
    QJsonObject obj;
    
    // 基本属性
    obj["entityType"] = item->data(0).toString();
    obj["entityId"] = item->data(1).toString();
    obj["typeName"] = item->data(2).toString();
    
    // 颜色和线宽
    QColor color = item->data(3).value<QColor>();
    if (color.isValid()) {
        obj["color"] = color.name();
    }
    obj["lineWidth"] = item->data(4).toInt();
    
    // 位置和层级
    obj["x"] = item->pos().x();
    obj["y"] = item->pos().y();
    obj["zValue"] = item->zValue();
    
    // 工具提示
    obj["toolTip"] = item->toolTip();
    
    // 图形类型特定数据
    if (auto pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item)) {
        obj["graphicsType"] = "path";
        
        // 序列化路径
        QPainterPath path = pathItem->path();
        QJsonArray pathArray;
        
        for (int i = 0; i < path.elementCount(); ++i) {
            QPainterPath::Element element = path.elementAt(i);
            QJsonObject elementObj;
            elementObj["type"] = static_cast<int>(element.type);
            elementObj["x"] = element.x;
            elementObj["y"] = element.y;
            pathArray.append(elementObj);
        }
        
        obj["path"] = pathArray;
        
        // 画笔
        QPen pen = pathItem->pen();
        obj["penColor"] = pen.color().name();
        obj["penWidth"] = pen.width();
        obj["penStyle"] = static_cast<int>(pen.style());
        
    } else if (auto ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
        obj["graphicsType"] = "ellipse";
        
        // 矩形
        QRectF rect = ellipseItem->rect();
        obj["rectX"] = rect.x();
        obj["rectY"] = rect.y();
        obj["rectWidth"] = rect.width();
        obj["rectHeight"] = rect.height();
        
        // 画笔和画刷
        QPen pen = ellipseItem->pen();
        obj["penColor"] = pen.color().name();
        obj["penWidth"] = pen.width();
        
        QBrush brush = ellipseItem->brush();
        obj["brushColor"] = brush.color().name();
    }
    
    return obj;
}

QGraphicsItem* DrawingDataManager::deserializeGraphicsItem(const QJsonObject &json)
{
    QString graphicsType = json["graphicsType"].toString();
    QGraphicsItem *item = nullptr;
    
    if (graphicsType == "path") {
        // 恢复路径项
        QGraphicsPathItem *pathItem = new QGraphicsPathItem();
        
        // 恢复路径
        QPainterPath path;
        QJsonArray pathArray = json["path"].toArray();
        
        for (const QJsonValue &value : pathArray) {
            QJsonObject elementObj = value.toObject();
            QPainterPath::ElementType type = static_cast<QPainterPath::ElementType>(elementObj["type"].toInt());
            double x = elementObj["x"].toDouble();
            double y = elementObj["y"].toDouble();
            
            if (type == QPainterPath::MoveToElement) {
                path.moveTo(x, y);
            } else if (type == QPainterPath::LineToElement) {
                path.lineTo(x, y);
            }
        }
        
        pathItem->setPath(path);
        
        // 恢复画笔
        QPen pen;
        pen.setColor(QColor(json["penColor"].toString()));
        pen.setWidth(json["penWidth"].toInt());
        pen.setStyle(static_cast<Qt::PenStyle>(json["penStyle"].toInt()));
        pathItem->setPen(pen);
        
        item = pathItem;
        
    } else if (graphicsType == "ellipse") {
        // 恢复椭圆项
        QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem();
        
        // 恢复矩形
        QRectF rect(
            json["rectX"].toDouble(),
            json["rectY"].toDouble(),
            json["rectWidth"].toDouble(),
            json["rectHeight"].toDouble()
        );
        ellipseItem->setRect(rect);
        
        // 恢复画笔和画刷
        QPen pen;
        pen.setColor(QColor(json["penColor"].toString()));
        pen.setWidth(json["penWidth"].toInt());
        ellipseItem->setPen(pen);
        
        QBrush brush;
        brush.setColor(QColor(json["brushColor"].toString()));
        brush.setStyle(Qt::SolidPattern);
        ellipseItem->setBrush(brush);
        
        item = ellipseItem;
    }
    
    if (item) {
        // 恢复基本属性
        item->setData(0, json["entityType"].toString());
        item->setData(1, json["entityId"].toString());
        item->setData(2, json["typeName"].toString());
        item->setData(3, QColor(json["color"].toString()));
        item->setData(4, json["lineWidth"].toInt());
        
        // 位置和层级
        item->setPos(json["x"].toDouble(), json["y"].toDouble());
        item->setZValue(json["zValue"].toDouble());
        
        // 工具提示
        item->setToolTip(json["toolTip"].toString());
    }
    
    return item;
}
