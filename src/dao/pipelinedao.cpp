#include "dao/pipelinedao.h"
#include "core/common/logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

PipelineDAO::PipelineDAO()
    : BaseDAO<Pipeline>("pipelines")
{
}

Pipeline PipelineDAO::fromQuery(QSqlQuery &query)
{
    Pipeline pipeline;

    pipeline.setId(query.value("id").toInt());
    pipeline.setPipelineId(query.value("pipeline_id").toString());
    pipeline.setPipelineName(query.value("pipeline_name").toString());
    pipeline.setPipelineType(query.value("pipeline_type").toString());

    // 几何信息 - 使用ST_AsText获取WKT格式
    QString geomWkt = query.value("geom_text").toString();
    pipeline.setGeomWkt(geomWkt);
    pipeline.setCoordinates(parseLineStringWkt(geomWkt));
    pipeline.setLengthM(query.value("length_m").toDouble());
    pipeline.setDepthM(query.value("depth_m").toDouble());

    // 物理属性
    pipeline.setDiameterMm(query.value("diameter_mm").toInt());
    pipeline.setMaterial(query.value("material").toString());
    pipeline.setPressureClass(query.value("pressure_class").toString());

    // 建设信息
    pipeline.setBuildDate(query.value("build_date").toDate());
    pipeline.setBuilder(query.value("builder").toString());
    pipeline.setOwner(query.value("owner").toString());
    pipeline.setConstructionCost(query.value("construction_cost").toDouble());

    // 运维信息
    pipeline.setStatus(query.value("status").toString());
    pipeline.setHealthScore(query.value("health_score").toInt());
    pipeline.setLastInspection(query.value("last_inspection").toDate());
    pipeline.setMaintenanceUnit(query.value("maintenance_unit").toString());
    pipeline.setInspectionCycle(query.value("inspection_cycle").toInt());

    // 元数据
    pipeline.setRemarks(query.value("remarks").toString());
    pipeline.setCreatedAt(query.value("created_at").toDateTime());
    pipeline.setUpdatedAt(query.value("updated_at").toDateTime());
    pipeline.setCreatedBy(query.value("created_by").toString());
    pipeline.setUpdatedBy(query.value("updated_by").toString());

    return pipeline;
}

QVariantMap PipelineDAO::toVariantMap(const Pipeline &pipeline)
{
    QVariantMap map;

    map["pipeline_id"] = pipeline.pipelineId();
    map["pipeline_name"] = pipeline.pipelineName();
    map["pipeline_type"] = pipeline.pipelineType();

    // 几何信息 - 使用ST_GeomFromText创建几何对象
    if (!pipeline.coordinates().isEmpty()) {
        QString wkt = toLineStringWkt(pipeline.coordinates());
        // 注意：这里需要使用PostgreSQL的函数，不能直接作为参数
        // 实际插入时需要特殊处理
        map["geom_wkt"] = wkt;
    }
    map["length_m"] = pipeline.lengthM();
    map["depth_m"] = pipeline.depthM();

    // 物理属性
    map["diameter_mm"] = pipeline.diameterMm();
    map["material"] = pipeline.material();
    map["pressure_class"] = pipeline.pressureClass();

    // 建设信息
    map["build_date"] = pipeline.buildDate();
    map["builder"] = pipeline.builder();
    map["owner"] = pipeline.owner();
    map["construction_cost"] = pipeline.constructionCost();

    // 运维信息
    map["status"] = pipeline.status();
    map["health_score"] = pipeline.healthScore();
    map["last_inspection"] = pipeline.lastInspection();
    map["maintenance_unit"] = pipeline.maintenanceUnit();
    map["inspection_cycle"] = pipeline.inspectionCycle();

    // 元数据
    map["remarks"] = pipeline.remarks();
    map["created_by"] = pipeline.createdBy();
    map["updated_by"] = pipeline.updatedBy();

    return map;
}

QVector<Pipeline> PipelineDAO::findByType(const QString &type, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE pipeline_type = :type LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":type"] = type;
    params[":limit"] = limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 pipelines of type: %2").arg(results.size()).arg(type));
    return results;
}

QVector<Pipeline> PipelineDAO::findByBounds(const QRectF &bounds, int limit)
{
    // 使用PostGIS的ST_MakeEnvelope和ST_Intersects进行空间查询
    QString sql = QString(
        "SELECT *, ST_AsText(geom) as geom_text "
        "FROM %1 "
        "WHERE ST_Intersects(geom, ST_MakeEnvelope(:minX, :minY, :maxX, :maxY, 4326)) "
        "LIMIT :limit"
    ).arg(m_tableName);

    QVariantMap params;
    params[":minX"] = bounds.left();
    params[":minY"] = bounds.bottom();
    params[":maxX"] = bounds.right();
    params[":maxY"] = bounds.top();
    params[":limit"] = limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 pipelines in bounds").arg(results.size()));
    return results;
}

Pipeline PipelineDAO::findByPipelineId(const QString &pipelineId)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE pipeline_id = :pipeline_id")
                      .arg(m_tableName);

    QVariantMap params;
    params[":pipeline_id"] = pipelineId;

    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    if (query.next()) {
        return fromQuery(query);
    }
    return Pipeline();
}

QVector<Pipeline> PipelineDAO::findByStatus(const QString &status, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE status = :status LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":status"] = status;
    params[":limit"] = limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    return results;
}

QVector<Pipeline> PipelineDAO::findByHealthScore(int maxScore, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE health_score <= :maxScore "
                          "ORDER BY health_score ASC LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":maxScore"] = maxScore;
    params[":limit"] = limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    return results;
}

QVector<Pipeline> PipelineDAO::findNearPoint(double lon, double lat, double radiusMeters, int limit)
{
    // 使用PostGIS的ST_DWithin进行缓冲区查询
    // ST_DWithin使用地理坐标系，单位为米
    QString sql = QString(
        "SELECT *, ST_AsText(geom) as geom_text, "
        "ST_Distance(geom::geography, ST_SetSRID(ST_MakePoint(:lon, :lat), 4326)::geography) as distance "
        "FROM %1 "
        "WHERE ST_DWithin(geom::geography, ST_SetSRID(ST_MakePoint(:lon, :lat), 4326)::geography, :radius) "
        "ORDER BY distance "
        "LIMIT :limit"
    ).arg(m_tableName);

    QVariantMap params;
    params[":lon"] = lon;
    params[":lat"] = lat;
    params[":radius"] = radiusMeters;
    params[":limit"] = limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 pipelines near point (%2, %3) within %4m")
                 .arg(results.size()).arg(lon).arg(lat).arg(radiusMeters));
    return results;
}

QMap<QString, int> PipelineDAO::countByType()
{
    QString sql = QString("SELECT pipeline_type, COUNT(*) as count FROM %1 GROUP BY pipeline_type")
                      .arg(m_tableName);

    QMap<QString, int> counts;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    while (query.next()) {
        QString type = query.value("pipeline_type").toString();
        int count = query.value("count").toInt();
        counts[type] = count;
    }

    return counts;
}

QVector<QPointF> PipelineDAO::parseLineStringWkt(const QString &wkt)
{
    QVector<QPointF> coordinates;

    // WKT格式: LINESTRING(lon1 lat1, lon2 lat2, ...)
    QString coordsStr = wkt;
    coordsStr.remove("LINESTRING(");
    coordsStr.remove(")");

    QStringList pairs = coordsStr.split(',', Qt::SkipEmptyParts);
    for (const QString &pair : pairs) {
        QStringList coords = pair.trimmed().split(' ', Qt::SkipEmptyParts);
        if (coords.size() == 2) {
            double lon = coords[0].toDouble();
            double lat = coords[1].toDouble();
            coordinates.append(QPointF(lon, lat));
        }
    }

    return coordinates;
}

QString PipelineDAO::toLineStringWkt(const QVector<QPointF> &coordinates)
{
    if (coordinates.isEmpty()) {
        return QString();
    }

    QStringList parts;
    for (const QPointF &point : coordinates) {
        parts.append(QString("%1 %2").arg(point.x()).arg(point.y()));
    }

    return QString("LINESTRING(%1)").arg(parts.join(", "));
}

