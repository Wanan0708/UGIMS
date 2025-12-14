#include "core/io/drawingdatabasemanager.h"
#include "core/database/databasemanager.h"
#include "core/common/logger.h"
#include "core/common/entitystate.h"  // 引入实体状态
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

bool DrawingDatabaseManager::saveToDatabase(QGraphicsScene *scene,
                                            const QHash<QGraphicsItem*, Pipeline> &pipelineHash)
{
    if (!scene) {
        qWarning() << "❌ Scene is null!";
        return false;
    }
    
    Logger::instance().info("开始增量保存绘制数据到数据库");
    
    qDebug() << "🔍 场景中总项数:" << scene->items().count();
    qDebug() << "🔍 pipelineHash大小:" << pipelineHash.size();
    
    int insertCount = 0;   // 插入数量
    int updateCount = 0;   // 更新数量
    int deleteCount = 0;   // 删除数量
    int unchangedCount = 0;  // 未变更数量
    int failCount = 0;     // 失败数量
    
    // 遍历场景中的所有项
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem *item : items) {
        QString entityType = item->data(0).toString();
        
        // 获取实体状态（存储在data(100)中）
        QVariant stateVariant = item->data(100);
        EntityState state = EntityState::Detached;
        if (stateVariant.isValid()) {
            state = static_cast<EntityState>(stateVariant.toInt());
        }
        
        qDebug() << "📊 实体:" << entityType << ", 状态:" << entityStateToString(state);
        
        // 根据状态进行不同操作
        if (state == EntityState::Unchanged || state == EntityState::Detached) {
            // 未变更或分离态，跳过
            unchangedCount++;
            continue;
        }
        
        // 处理管线
        if (entityType == "pipeline") {
            auto pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item);
            if (pathItem && pipelineHash.contains(item)) {
                Pipeline pipeline = pipelineHash[item];
                
                if (state == EntityState::Added) {
                    // 新增：执行INSERT
                    if (insertPipelineToDatabase(pathItem, pipeline)) {
                        insertCount++;
                        // 更新状态为 Unchanged
                        item->setData(100, static_cast<int>(EntityState::Unchanged));
                    } else {
                        failCount++;
                    }
                } else if (state == EntityState::Modified) {
                    // 修改：执行UPDATE
                    if (updatePipelineToDatabase(pathItem, pipeline)) {
                        updateCount++;
                        // 更新状态为 Unchanged
                        item->setData(100, static_cast<int>(EntityState::Unchanged));
                    } else {
                        failCount++;
                    }
                } else if (state == EntityState::Deleted) {
                    // 删除：执行DELETE
                    if (deletePipelineFromDatabase(pipeline.pipelineId())) {
                        deleteCount++;
                        // 从场景中移除
                        scene->removeItem(item);
                        delete item;
                    } else {
                        failCount++;
                    }
                }
            }
        }
        // 处理设施
        else if (entityType == "facility") {
            auto ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
            if (ellipseItem) {
                QString facilityId = item->data(10).toString();
                
                if (state == EntityState::Added) {
                    // 新增：执行INSERT
                    if (insertFacilityToDatabase(ellipseItem)) {
                        insertCount++;
                        // 更新状态为 Unchanged
                        item->setData(100, static_cast<int>(EntityState::Unchanged));
                    } else {
                        failCount++;
                    }
                } else if (state == EntityState::Modified) {
                    // 修改：执行UPDATE
                    if (updateFacilityToDatabase(ellipseItem)) {
                        updateCount++;
                        // 更新状态为 Unchanged
                        item->setData(100, static_cast<int>(EntityState::Unchanged));
                    } else {
                        failCount++;
                    }
                } else if (state == EntityState::Deleted) {
                    // 删除：执行DELETE
                    if (deleteFacilityFromDatabase(facilityId)) {
                        deleteCount++;
                        // 从场景中移除
                        scene->removeItem(item);
                        delete item;
                    } else {
                        failCount++;
                    }
                }
            }
        }
    }
    
    QString msg = QString("增量保存完成：新增=%1, 更新=%2, 删除=%3, 未变更=%4, 失败=%5")
        .arg(insertCount).arg(updateCount).arg(deleteCount).arg(unchangedCount).arg(failCount);
    Logger::instance().info(msg);
    qDebug() << msg;
    
    return (insertCount + updateCount + deleteCount > 0);
}

bool DrawingDatabaseManager::loadFromDatabase(QGraphicsScene *scene,
                                              QHash<QGraphicsItem*, Pipeline> &pipelineHash,
                                              int &nextId)
{
    if (!scene) {
        qWarning() << "Scene is null!";
        return false;
    }
    
    Logger::instance().info("开始从数据库加载绘制数据");
    
    // 加载管线
    int pipelineCount = loadPipelinesFromDatabase(scene, pipelineHash);
    
    // 加载设施
    int facilityCount = loadFacilitiesFromDatabase(scene);
    
    // 更新nextId（查找最大ID）
    int maxId = 0;
    for (const Pipeline &pipeline : pipelineHash.values()) {
        QString pipelineId = pipeline.pipelineId();
        // 假设ID格式为 "PIPELINE-XXX"，提取数字部分
        QStringList parts = pipelineId.split('-');
        if (parts.size() >= 2) {
            bool ok;
            int id = parts.last().toInt(&ok);
            if (ok && id > maxId) {
                maxId = id;
            }
        }
    }
    nextId = maxId + 1;
    
    QString msg = QString("加载完成：%1条管线，%2个设施").arg(pipelineCount).arg(facilityCount);
    Logger::instance().info(msg);
    qDebug() << msg;
    
    return (pipelineCount > 0 || facilityCount > 0);
}

bool DrawingDatabaseManager::clearDatabase()
{
    Logger::instance().info("清空数据库中的绘制数据");
    
    // 删除所有created_by='user_drawing'的记录（用户绘制的管线）
    QString sql1 = "DELETE FROM pipelines WHERE created_by = 'user_drawing'";
    bool result1 = DatabaseManager::instance().executeCommand(sql1);
    
    // 删除所有created_by='user_drawing'的记录（用户绘制的设施）
    QString sql2 = "DELETE FROM facilities WHERE created_by = 'user_drawing'";
    bool result2 = DatabaseManager::instance().executeCommand(sql2);
    
    return result1 && result2;
}

bool DrawingDatabaseManager::insertPipelineToDatabase(QGraphicsPathItem *pathItem, const Pipeline &pipeline)
{
    if (!pathItem) {
        qWarning() << "❌ pathItem is null";
        return false;
    }
    
    // 将QPainterPath转换为WKT格式
    QString wkt = painterPathToWkt(pathItem->path());
    
    qDebug() << "➕ INSERT 管线:" << pipeline.pipelineId();
    
    // 使用PostGIS的ST_GeomFromText函数
    QString sql = "INSERT INTO pipelines ("
                 "pipeline_id, pipeline_name, pipeline_type, geom, "
                 "diameter_mm, material, status, health_score, "
                 "created_at, created_by) "
                 "VALUES ("
                 ":pipeline_id, :pipeline_name, :pipeline_type, "
                 "ST_GeomFromText(:geom_wkt, 4326), "
                 ":diameter_mm, :material, :status, :health_score, "
                 ":created_at, :created_by)";
    
    QVariantMap params;
    params[":pipeline_id"] = pipeline.pipelineId();
    params[":pipeline_name"] = pipeline.pipelineName();
    params[":pipeline_type"] = pipeline.pipelineType();
    params[":geom_wkt"] = wkt;
    params[":diameter_mm"] = pathItem->data(3).toInt();
    params[":material"] = "unknown";
    params[":status"] = "active";
    params[":health_score"] = 100;
    params[":created_at"] = QDateTime::currentDateTime();
    params[":created_by"] = "user_drawing";
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        qDebug() << "✅ INSERT 管线成功:" << pipeline.pipelineId();
    } else {
        QString error = DatabaseManager::instance().lastError();
        qWarning() << "❌ INSERT 管线失败:" << pipeline.pipelineId();
        qWarning() << "❌ 错误:" << error;
    }
    
    return success;
}

bool DrawingDatabaseManager::updatePipelineToDatabase(QGraphicsPathItem *pathItem, const Pipeline &pipeline)
{
    if (!pathItem) {
        qWarning() << "❌ pathItem is null";
        return false;
    }
    
    // 将QPainterPath转换为WKT格式
    QString wkt = painterPathToWkt(pathItem->path());
    
    qDebug() << "🔄 UPDATE 管线:" << pipeline.pipelineId();
    
    QString sql = "UPDATE pipelines SET "
                 "pipeline_name = :pipeline_name, "
                 "pipeline_type = :pipeline_type, "
                 "geom = ST_GeomFromText(:geom_wkt, 4326), "
                 "diameter_mm = :diameter_mm, "
                 "updated_at = :updated_at "
                 "WHERE pipeline_id = :pipeline_id";
    
    QVariantMap params;
    params[":pipeline_id"] = pipeline.pipelineId();
    params[":pipeline_name"] = pipeline.pipelineName();
    params[":pipeline_type"] = pipeline.pipelineType();
    params[":geom_wkt"] = wkt;
    params[":diameter_mm"] = pathItem->data(3).toInt();
    params[":updated_at"] = QDateTime::currentDateTime();
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        qDebug() << "✅ UPDATE 管线成功:" << pipeline.pipelineId();
    } else {
        qWarning() << "❌ UPDATE 管线失败:" << pipeline.pipelineId();
    }
    
    return success;
}

bool DrawingDatabaseManager::deletePipelineFromDatabase(const QString &pipelineId)
{
    qDebug() << "🗑️ DELETE 管线:" << pipelineId;
    
    QString sql = "DELETE FROM pipelines WHERE pipeline_id = :pipeline_id";
    
    QVariantMap params;
    params[":pipeline_id"] = pipelineId;
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        qDebug() << "✅ DELETE 管线成功:" << pipelineId;
    } else {
        qWarning() << "❌ DELETE 管线失败:" << pipelineId;
    }
    
    return success;
}

bool DrawingDatabaseManager::insertFacilityToDatabase(QGraphicsEllipseItem *ellipseItem)
{
    if (!ellipseItem) {
        return false;
    }
    
    // 生成设施ID
    static int facilityCounter = 1;
    QString facilityId = QString("FACILITY-%1").arg(facilityCounter++, 3, 10, QChar('0'));
    
    // 获取设施类型
    QString facilityType = ellipseItem->data(1).toString();
    if (facilityType.isEmpty()) {
        facilityType = "unknown";
    }
    
    // 获取中心点坐标
    QRectF rect = ellipseItem->rect();
    QPointF center = ellipseItem->pos() + rect.center();
    
    qDebug() << "➕ INSERT 设施:" << facilityId;
    
    // 构建SQL语句
    QString sql = "INSERT INTO facilities ("
                 "facility_id, facility_name, facility_type, geom, "
                 "status, health_score, created_at, created_by) "
                 "VALUES ("
                 ":facility_id, :facility_name, :facility_type, "
                 "ST_GeomFromText(:geom_wkt, 4326), "
                 ":status, :health_score, :created_at, :created_by)";
    
    QVariantMap params;
    params[":facility_id"] = facilityId;
    params[":facility_name"] = facilityType + " " + facilityId;
    params[":facility_type"] = facilityType;
    params[":geom_wkt"] = pointToWkt(center);
    params[":status"] = "normal";
    params[":health_score"] = 100;
    params[":created_at"] = QDateTime::currentDateTime();
    params[":created_by"] = "user_drawing";
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        // 保存成功后，将ID存储到item中
        ellipseItem->setData(10, facilityId);
        qDebug() << "✅ INSERT 设施成功:" << facilityId;
    } else {
        qWarning() << "❌ INSERT 设施失败:" << facilityId;
    }
    
    return success;
}

bool DrawingDatabaseManager::updateFacilityToDatabase(QGraphicsEllipseItem *ellipseItem)
{
    if (!ellipseItem) {
        return false;
    }
    
    QString facilityId = ellipseItem->data(10).toString();
    QString facilityType = ellipseItem->data(1).toString();
    
    // 获取中心点坐标
    QRectF rect = ellipseItem->rect();
    QPointF center = ellipseItem->pos() + rect.center();
    
    qDebug() << "🔄 UPDATE 设施:" << facilityId;
    
    QString sql = "UPDATE facilities SET "
                 "facility_type = :facility_type, "
                 "geom = ST_GeomFromText(:geom_wkt, 4326), "
                 "updated_at = :updated_at "
                 "WHERE facility_id = :facility_id";
    
    QVariantMap params;
    params[":facility_id"] = facilityId;
    params[":facility_type"] = facilityType;
    params[":geom_wkt"] = pointToWkt(center);
    params[":updated_at"] = QDateTime::currentDateTime();
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        qDebug() << "✅ UPDATE 设施成功:" << facilityId;
    } else {
        qWarning() << "❌ UPDATE 设施失败:" << facilityId;
    }
    
    return success;
}

bool DrawingDatabaseManager::deleteFacilityFromDatabase(const QString &facilityId)
{
    qDebug() << "🗑️ DELETE 设施:" << facilityId;
    
    QString sql = "DELETE FROM facilities WHERE facility_id = :facility_id";
    
    QVariantMap params;
    params[":facility_id"] = facilityId;
    
    bool success = DatabaseManager::instance().executeCommand(sql, params);
    
    if (success) {
        qDebug() << "✅ DELETE 设施成功:" << facilityId;
    } else {
        qWarning() << "❌ DELETE 设施失败:" << facilityId;
    }
    
    return success;
}

int DrawingDatabaseManager::loadPipelinesFromDatabase(QGraphicsScene *scene,
                                                      QHash<QGraphicsItem*, Pipeline> &pipelineHash)
{
    // 查询所有用户绘制的管线（使用created_by字段）
    QString sql = "SELECT *, ST_AsText(geom) as geom_text "
                 "FROM pipelines "
                 "WHERE created_by = 'user_drawing' "
                 "ORDER BY created_at";
    
    qDebug() << "🔍 执行加载SQL:" << sql;
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    
    if (query.lastError().isValid()) {
        qWarning() << "❌ 查询失败:" << query.lastError().text();
        qWarning() << "❌ SQL:" << sql;
        return 0;
    }
    
    int count = 0;
    while (query.next()) {
        // 解析管线数据
        Pipeline pipeline;
        pipeline.setId(query.value("id").toInt());
        pipeline.setPipelineId(query.value("pipeline_id").toString());
        pipeline.setPipelineName(query.value("pipeline_name").toString());
        pipeline.setPipelineType(query.value("pipeline_type").toString());
        
        // 从 ST_AsText(geom) 结果读取WKT
        QString geomWkt = query.value("geom_text").toString();
        pipeline.setGeomWkt(geomWkt);
        
        pipeline.setDiameterMm(query.value("diameter_mm").toInt());
        pipeline.setMaterial(query.value("material").toString());
        pipeline.setStatus(query.value("status").toString());
        pipeline.setHealthScore(query.value("health_score").toInt());
        
        // 将WKT转换为QPainterPath
        QPainterPath path = wktToPainterPath(geomWkt);
        
        // 创建QGraphicsPathItem
        QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
        
        // 设置样式（根据管线类型设置颜色）
        QColor color = Qt::blue;  // 默认颜色
        QString pipelineType = pipeline.pipelineType();
        if (pipelineType == "water_supply") {
            color = QColor(0, 112, 192);  // 蓝色
        } else if (pipelineType == "sewage") {
            color = QColor(112, 48, 160);  // 紫色
        } else if (pipelineType == "gas") {
            color = QColor(255, 192, 0);   // 黄色
        } else if (pipelineType == "electric") {
            color = QColor(255, 0, 0);     // 红色
        } else if (pipelineType == "telecom") {
            color = QColor(0, 176, 80);    // 绿色
        } else if (pipelineType == "heat") {
            color = QColor(255, 128, 0);   // 橙色
        }
        
        int lineWidth = query.value("diameter_mm").toInt();
        if (lineWidth <= 0) lineWidth = 2;
        
        QPen pen(color, lineWidth);
        pathItem->setPen(pen);
        
        // 设置数据（与PipelineRenderer保持一致）
        pathItem->setData(0, "pipeline");  // 实体类型
        pathItem->setData(1, pipeline.pipelineId());  // 管线编号
        pathItem->setData(2, pipeline.pipelineType());  // 管线类型（用于图层控制）
        pathItem->setData(3, lineWidth);  // 线宽
        pathItem->setData(10, pipeline.id());  // 数据库ID
        pathItem->setData(100, static_cast<int>(EntityState::Unchanged));  // 实体状态：未变更
        
        // 设置工具提示
        pathItem->setToolTip(QString("%1\nID: %2\n类型: %3")
                            .arg(pipeline.pipelineName())
                            .arg(pipeline.pipelineId())
                            .arg(pipeline.pipelineType()));
        
        // 添加到场景
        scene->addItem(pathItem);
        
        // 添加到哈希表
        pipelineHash[pathItem] = pipeline;
        
        count++;
    }
    
    qDebug() << "从数据库加载管线数量:" << count;
    return count;
}

int DrawingDatabaseManager::loadFacilitiesFromDatabase(QGraphicsScene *scene)
{
    // 查询所有用户绘制的设施（使用created_by字段）
    QString sql = "SELECT *, ST_AsText(geom) as geom_text "
                 "FROM facilities "
                 "WHERE created_by = 'user_drawing' "
                 "ORDER BY created_at";
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    
    int count = 0;
    while (query.next()) {
        // 解析设施数据
        QString facilityId = query.value("facility_id").toString();
        QString facilityType = query.value("facility_type").toString();
        QString geomWkt = query.value("geom_text").toString();
        
        // 将WKT转换为点坐标
        QPointF center = wktToPoint(geomWkt);
        
        // 创建QGraphicsEllipseItem
        double radius = 5.0;  // 默认半径
        QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem(
            -radius, -radius, radius * 2, radius * 2
        );
        
        ellipseItem->setPos(center);
        
        // 设置样式
        QColor color = Qt::red;  // 默认颜色
        ellipseItem->setBrush(QBrush(color));
        ellipseItem->setPen(QPen(Qt::black, 1));
        
        // 设置数据（与FacilityRenderer保持一致）
        ellipseItem->setData(0, "facility");  // 实体类型
        ellipseItem->setData(1, facilityId);  // 设施编号
        ellipseItem->setData(2, facilityType);  // 设施类型（用于图层控制）
        ellipseItem->setData(10, facilityId);  // 设施ID
        ellipseItem->setData(100, static_cast<int>(EntityState::Unchanged));  // 实体状态：未变更
        
        // 设置可选中和可交互标志（重要：使设施可以被点击选中）
        ellipseItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        ellipseItem->setFlag(QGraphicsItem::ItemIsFocusable, true);
        ellipseItem->setAcceptHoverEvents(true);
        
        // 设置工具提示
        ellipseItem->setToolTip(QString("设施: %1\nID: %2\n类型: %3")
                               .arg(facilityType)
                               .arg(facilityId)
                               .arg(facilityType));
        
        // 添加到场景
        scene->addItem(ellipseItem);
        
        count++;
    }
    
    qDebug() << "从数据库加载设施数量:" << count;
    return count;
}

QString DrawingDatabaseManager::painterPathToWkt(const QPainterPath &path)
{
    if (path.elementCount() == 0) {
        return QString();
    }
    
    QStringList coords;
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element element = path.elementAt(i);
        // WKT格式：LINESTRING(x1 y1, x2 y2, ...)
        coords.append(QString("%1 %2").arg(element.x, 0, 'f', 6).arg(element.y, 0, 'f', 6));
    }
    
    return QString("LINESTRING(%1)").arg(coords.join(", "));
}

QPainterPath DrawingDatabaseManager::wktToPainterPath(const QString &wkt)
{
    QPainterPath path;
    
    // 解析WKT格式：LINESTRING(x1 y1, x2 y2, ...)
    QString coordsStr = wkt;
    coordsStr.remove("LINESTRING(");
    coordsStr.remove(")");
    
    QStringList coordPairs = coordsStr.split(',', Qt::SkipEmptyParts);
    
    bool isFirst = true;
    for (const QString &coordPair : coordPairs) {
        QStringList coords = coordPair.trimmed().split(' ', Qt::SkipEmptyParts);
        if (coords.size() >= 2) {
            double x = coords[0].toDouble();
            double y = coords[1].toDouble();
            
            if (isFirst) {
                path.moveTo(x, y);
                isFirst = false;
            } else {
                path.lineTo(x, y);
            }
        }
    }
    
    return path;
}

QString DrawingDatabaseManager::pointToWkt(const QPointF &point)
{
    // WKT格式：POINT(x y)
    return QString("POINT(%1 %2)")
        .arg(point.x(), 0, 'f', 6)
        .arg(point.y(), 0, 'f', 6);
}

QPointF DrawingDatabaseManager::wktToPoint(const QString &wkt)
{
    // 解析WKT格式：POINT(x y)
    QString coordsStr = wkt;
    coordsStr.remove("POINT(");
    coordsStr.remove(")");
    
    QStringList coords = coordsStr.trimmed().split(' ', Qt::SkipEmptyParts);
    if (coords.size() >= 2) {
        double x = coords[0].toDouble();
        double y = coords[1].toDouble();
        return QPointF(x, y);
    }
    
    return QPointF();
}
