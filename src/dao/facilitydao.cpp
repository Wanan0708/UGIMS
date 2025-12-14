#include "dao/facilitydao.h"
#include "core/common/logger.h"
#include "core/database/databasemanager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

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

// 转义SQL字符串值，防止SQL注入
static QString escapeSqlString(const QString &str)
{
    QString escaped = str;
    escaped.replace("'", "''");
    return escaped;
}

bool FacilityDAO::insert(const Facility &facility)
{
    QVariantMap data = toVariantMap(facility);
    
    // 手动构建SQL，避免PostGIS函数与参数绑定冲突
    QStringList columns;
    QStringList values;
    
    // 转义SQL字符串值，防止SQL注入
    auto escapeSqlString = [](const QString &str) -> QString {
        QString escaped = str;
        escaped.replace("'", "''");
        return escaped;
    };
    
    // 处理各个字段
    if (data.contains("facility_id") && !data.value("facility_id").toString().isEmpty()) {
        columns.append("facility_id");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("facility_id").toString())));
    }
    
    if (data.contains("facility_name") && !data.value("facility_name").toString().isEmpty()) {
        columns.append("facility_name");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("facility_name").toString())));
    }
    
    if (data.contains("facility_type") && !data.value("facility_type").toString().isEmpty()) {
        columns.append("facility_type");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("facility_type").toString())));
    }
    
    // 处理几何字段 - 使用ST_GeomFromText
    QString geomWkt = data.value("geom_wkt").toString();
    if (!geomWkt.isEmpty()) {
        columns.append("geom");
        QString escaped = geomWkt;
        escaped.replace("'", "''");
        values.append(QString("ST_GeomFromText('%1', 4326)").arg(escaped));
    }
    
    if (data.contains("elevation_m")) {
        columns.append("elevation_m");
        values.append(QString::number(data.value("elevation_m").toDouble(), 'f', 2));
    }
    
    if (data.contains("spec") && !data.value("spec").toString().isEmpty()) {
        columns.append("spec");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("spec").toString())));
    }
    
    if (data.contains("material") && !data.value("material").toString().isEmpty()) {
        columns.append("material");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("material").toString())));
    }
    
    if (data.contains("size") && !data.value("size").toString().isEmpty()) {
        columns.append("size");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("size").toString())));
    }
    
    if (data.contains("pipeline_id") && !data.value("pipeline_id").toString().isEmpty()) {
        columns.append("pipeline_id");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("pipeline_id").toString())));
    }
    
    if (data.contains("build_date") && data.value("build_date").toDate().isValid() && 
        data.value("build_date").toDate().year() > 2000) {
        columns.append("build_date");
        values.append(QString("'%1'").arg(data.value("build_date").toDate().toString("yyyy-MM-dd")));
    }
    
    if (data.contains("builder") && !data.value("builder").toString().isEmpty()) {
        columns.append("builder");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("builder").toString())));
    }
    
    if (data.contains("owner") && !data.value("owner").toString().isEmpty()) {
        columns.append("owner");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("owner").toString())));
    }
    
    if (data.contains("asset_value")) {
        columns.append("asset_value");
        values.append(QString::number(data.value("asset_value").toDouble(), 'f', 2));
    }
    
    if (data.contains("status") && !data.value("status").toString().isEmpty()) {
        columns.append("status");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("status").toString())));
    }
    
    if (data.contains("health_score")) {
        columns.append("health_score");
        values.append(QString::number(data.value("health_score").toInt()));
    }
    
    if (data.contains("last_maintenance") && data.value("last_maintenance").toDate().isValid() && 
        data.value("last_maintenance").toDate().year() > 2000) {
        columns.append("last_maintenance");
        values.append(QString("'%1'").arg(data.value("last_maintenance").toDate().toString("yyyy-MM-dd")));
    }
    
    if (data.contains("next_maintenance") && data.value("next_maintenance").toDate().isValid() && 
        data.value("next_maintenance").toDate().year() > 2000) {
        columns.append("next_maintenance");
        values.append(QString("'%1'").arg(data.value("next_maintenance").toDate().toString("yyyy-MM-dd")));
    }
    
    if (data.contains("maintenance_unit") && !data.value("maintenance_unit").toString().isEmpty()) {
        columns.append("maintenance_unit");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("maintenance_unit").toString())));
    }
    
    if (data.contains("qrcode_url") && !data.value("qrcode_url").toString().isEmpty()) {
        columns.append("qrcode_url");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("qrcode_url").toString())));
    }
    
    if (data.contains("remarks") && !data.value("remarks").toString().isEmpty()) {
        columns.append("remarks");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("remarks").toString())));
    }
    
    if (data.contains("created_by") && !data.value("created_by").toString().isEmpty()) {
        columns.append("created_by");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("created_by").toString())));
    }
    
    if (data.contains("updated_by") && !data.value("updated_by").toString().isEmpty()) {
        columns.append("updated_by");
        values.append(QString("'%1'").arg(escapeSqlString(data.value("updated_by").toString())));
    }
    
    // 添加时间戳
    columns.append("created_at");
    values.append("CURRENT_TIMESTAMP");
    columns.append("updated_at");
    values.append("CURRENT_TIMESTAMP");
    
    if (columns.isEmpty()) {
        qDebug() << "[FacilityDAO] Insert failed: no fields to insert";
        return false;
    }
    
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(m_tableName)
                      .arg(columns.join(", "))
                      .arg(values.join(", "));
    
    qDebug() << "[FacilityDAO] Insert SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[FacilityDAO] Insert failed:" << error;
        LOG_ERROR(QString("Facility insert failed: %1").arg(error));
    } else {
        qDebug() << "[FacilityDAO] Insert successful";
    }
    return result;
}

bool FacilityDAO::update(const Facility &facility, int id)
{
    if (id <= 0) {
        qDebug() << "[FacilityDAO] Update failed: invalid facility ID";
        return false;
    }
    
    QVariantMap data = toVariantMap(facility);
    
    // 完全手动构建SQL，避免PostGIS函数与参数绑定冲突
    QStringList setParts;
    
    // 更新字段
    if (!data.value("facility_name").toString().isEmpty()) {
        setParts.append(QString("facility_name = '%1'").arg(escapeSqlString(data.value("facility_name").toString())));
    }
    
    if (!data.value("facility_type").toString().isEmpty()) {
        setParts.append(QString("facility_type = '%1'").arg(escapeSqlString(data.value("facility_type").toString())));
    }
    
    if (data.value("elevation_m").toDouble() != 0.0) {
        setParts.append(QString("elevation_m = %1").arg(data.value("elevation_m").toDouble(), 0, 'f', 2));
    }
    
    if (!data.value("spec").toString().isEmpty()) {
        setParts.append(QString("spec = '%1'").arg(escapeSqlString(data.value("spec").toString())));
    }
    
    if (!data.value("material").toString().isEmpty()) {
        setParts.append(QString("material = '%1'").arg(escapeSqlString(data.value("material").toString())));
    }
    
    if (!data.value("pipeline_id").toString().isEmpty()) {
        setParts.append(QString("pipeline_id = '%1'").arg(escapeSqlString(data.value("pipeline_id").toString())));
    }
    
    if (data.value("build_date").toDate().isValid() && data.value("build_date").toDate().year() > 2000) {
        setParts.append(QString("build_date = '%1'").arg(data.value("build_date").toDate().toString("yyyy-MM-dd")));
    }
    
    if (!data.value("builder").toString().isEmpty()) {
        setParts.append(QString("builder = '%1'").arg(escapeSqlString(data.value("builder").toString())));
    }
    
    if (!data.value("owner").toString().isEmpty()) {
        setParts.append(QString("owner = '%1'").arg(escapeSqlString(data.value("owner").toString())));
    }
    
    if (!data.value("status").toString().isEmpty()) {
        setParts.append(QString("status = '%1'").arg(escapeSqlString(data.value("status").toString())));
    }
    
    setParts.append(QString("health_score = %1").arg(data.value("health_score").toInt()));
    
    if (data.value("last_maintenance").toDate().isValid() && data.value("last_maintenance").toDate().year() > 2000) {
        setParts.append(QString("last_maintenance = '%1'").arg(data.value("last_maintenance").toDate().toString("yyyy-MM-dd")));
    }
    
    if (data.value("next_maintenance").toDate().isValid() && data.value("next_maintenance").toDate().year() > 2000) {
        setParts.append(QString("next_maintenance = '%1'").arg(data.value("next_maintenance").toDate().toString("yyyy-MM-dd")));
    }
    
    if (!data.value("maintenance_unit").toString().isEmpty()) {
        setParts.append(QString("maintenance_unit = '%1'").arg(escapeSqlString(data.value("maintenance_unit").toString())));
    }
    
    if (!data.value("remarks").toString().isEmpty()) {
        setParts.append(QString("remarks = '%1'").arg(escapeSqlString(data.value("remarks").toString())));
    }
    
    // 处理几何字段
    QString geomWkt = data.value("geom_wkt").toString();
    if (!geomWkt.isEmpty()) {
        QString escaped = geomWkt;
        escaped.replace("'", "''");
        setParts.append(QString("geom = ST_GeomFromText('%1', 4326)").arg(escaped));
    }
    
    setParts.append("updated_at = CURRENT_TIMESTAMP");
    
    if (setParts.isEmpty()) {
        qDebug() << "[FacilityDAO] No fields to update";
        return false;
    }
    
    QString sql = QString("UPDATE %1 SET %2 WHERE id = %3")
                      .arg(m_tableName)
                      .arg(setParts.join(", "))
                      .arg(id);
    
    qDebug() << "[FacilityDAO] Update SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[FacilityDAO] Update failed:" << error;
    }
    return result;
}

