#include "dao/facilitydao.h"
#include "core/common/logger.h"
#include <QSqlQuery>
#include <QVariant>

FacilityDAO::FacilityDAO()
    : BaseDAO<Facility>("facilities")
{
}

Facility FacilityDAO::fromQuery(QSqlQuery &query)
{
    Facility facility;

    facility.setId(query.value("id").toInt());
    facility.setFacilityId(query.value("facility_id").toString());
    facility.setFacilityName(query.value("facility_name").toString());
    facility.setFacilityType(query.value("facility_type").toString());

    // 几何信息
    QString geomWkt = query.value("geom_text").toString();
    facility.setGeomWkt(geomWkt);
    facility.setCoordinate(parsePointWkt(geomWkt));
    facility.setElevationM(query.value("elevation_m").toDouble());

    // 物理属性
    facility.setSpec(query.value("spec").toString());
    facility.setMaterial(query.value("material").toString());
    facility.setSize(query.value("size").toString());

    // 关联管线
    facility.setPipelineId(query.value("pipeline_id").toString());

    // 建设信息
    facility.setBuildDate(query.value("build_date").toDate());
    facility.setBuilder(query.value("builder").toString());
    facility.setOwner(query.value("owner").toString());
    facility.setAssetValue(query.value("asset_value").toDouble());

    // 运维信息
    facility.setStatus(query.value("status").toString());
    facility.setHealthScore(query.value("health_score").toInt());
    facility.setLastMaintenance(query.value("last_maintenance").toDate());
    facility.setNextMaintenance(query.value("next_maintenance").toDate());
    facility.setMaintenanceUnit(query.value("maintenance_unit").toString());

    // 二维码
    facility.setQrcodeUrl(query.value("qrcode_url").toString());

    // 元数据
    facility.setRemarks(query.value("remarks").toString());
    facility.setCreatedAt(query.value("created_at").toDateTime());
    facility.setUpdatedAt(query.value("updated_at").toDateTime());
    facility.setCreatedBy(query.value("created_by").toString());
    facility.setUpdatedBy(query.value("updated_by").toString());

    return facility;
}

QVariantMap FacilityDAO::toVariantMap(const Facility &facility)
{
    QVariantMap map;

    map["facility_id"] = facility.facilityId();
    map["facility_name"] = facility.facilityName();
    map["facility_type"] = facility.facilityType();

    // 几何信息
    if (!facility.coordinate().isNull()) {
        map["geom_wkt"] = toPointWkt(facility.coordinate());
    }
    map["elevation_m"] = facility.elevationM();

    // 物理属性
    map["spec"] = facility.spec();
    map["material"] = facility.material();
    map["size"] = facility.size();

    // 关联管线
    map["pipeline_id"] = facility.pipelineId();

    // 建设信息
    map["build_date"] = facility.buildDate();
    map["builder"] = facility.builder();
    map["owner"] = facility.owner();
    map["asset_value"] = facility.assetValue();

    // 运维信息
    map["status"] = facility.status();
    map["health_score"] = facility.healthScore();
    map["last_maintenance"] = facility.lastMaintenance();
    map["next_maintenance"] = facility.nextMaintenance();
    map["maintenance_unit"] = facility.maintenanceUnit();

    // 二维码
    map["qrcode_url"] = facility.qrcodeUrl();

    // 元数据
    map["remarks"] = facility.remarks();
    map["created_by"] = facility.createdBy();
    map["updated_by"] = facility.updatedBy();

    return map;
}

QVector<Facility> FacilityDAO::findAll(int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":limit"] = limit;

    qDebug() << "[FacilityDAO] findAll called, limit:" << limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        qDebug() << "[FacilityDAO] Query error:" << query.lastError().text();
        LOG_ERROR(QString("Facility query failed: %1").arg(query.lastError().text()));
    }
    
    while (query.next()) {
        results.append(fromQuery(query));
    }

    qDebug() << "[FacilityDAO] Found" << results.size() << "facilities";
    LOG_INFO(QString("Found %1 facilities").arg(results.size()));
    return results;
}

QVector<Facility> FacilityDAO::findByType(const QString &type, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE facility_type = :type LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":type"] = type;
    params[":limit"] = limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 facilities of type: %2").arg(results.size()).arg(type));
    return results;
}

QVector<Facility> FacilityDAO::findByBounds(const QRectF &bounds, int limit)
{
    QString sql = QString(
        "SELECT *, ST_AsText(geom) as geom_text "
        "FROM %1 "
        "WHERE ST_Within(geom, ST_MakeEnvelope(:minX, :minY, :maxX, :maxY, 4326)) "
        "LIMIT :limit"
    ).arg(m_tableName);

    QVariantMap params;
    params[":minX"] = bounds.left();
    params[":minY"] = bounds.bottom();
    params[":maxX"] = bounds.right();
    params[":maxY"] = bounds.top();
    params[":limit"] = limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 facilities in bounds").arg(results.size()));
    return results;
}

Facility FacilityDAO::findByFacilityId(const QString &facilityId)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE facility_id = :facility_id")
                      .arg(m_tableName);

    QVariantMap params;
    params[":facility_id"] = facilityId;

    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    if (query.next()) {
        return fromQuery(query);
    }
    return Facility();
}

QVector<Facility> FacilityDAO::findByPipelineId(const QString &pipelineId, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE pipeline_id = :pipeline_id LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":pipeline_id"] = pipelineId;
    params[":limit"] = limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    return results;
}

QVector<Facility> FacilityDAO::findByStatus(const QString &status, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE status = :status LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":status"] = status;
    params[":limit"] = limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    return results;
}

QVector<Facility> FacilityDAO::findByHealthScore(int maxScore, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE health_score <= :maxScore "
                          "ORDER BY health_score ASC LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":maxScore"] = maxScore;
    params[":limit"] = limit;

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    return results;
}

QVector<Facility> FacilityDAO::findNearPoint(double lon, double lat, double radiusMeters, int limit)
{
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

    QVector<Facility> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }

    LOG_INFO(QString("Found %1 facilities near point (%2, %3) within %4m")
                 .arg(results.size()).arg(lon).arg(lat).arg(radiusMeters));
    return results;
}

QMap<QString, int> FacilityDAO::countByType()
{
    QString sql = QString("SELECT facility_type, COUNT(*) as count FROM %1 GROUP BY facility_type")
                      .arg(m_tableName);

    QMap<QString, int> counts;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    while (query.next()) {
        QString type = query.value("facility_type").toString();
        int count = query.value("count").toInt();
        counts[type] = count;
    }

    return counts;
}

QPointF FacilityDAO::parsePointWkt(const QString &wkt)
{
    // WKT格式: POINT(lon lat)
    QString coordsStr = wkt;
    coordsStr.remove("POINT(");
    coordsStr.remove(")");

    QStringList coords = coordsStr.split(' ', Qt::SkipEmptyParts);
    if (coords.size() == 2) {
        double lon = coords[0].toDouble();
        double lat = coords[1].toDouble();
        return QPointF(lon, lat);
    }

    return QPointF();
}

QString FacilityDAO::toPointWkt(const QPointF &coordinate)
{
    return QString("POINT(%1 %2)").arg(coordinate.x()).arg(coordinate.y());
}

