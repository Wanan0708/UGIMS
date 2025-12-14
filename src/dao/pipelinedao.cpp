#include "dao/pipelinedao.h"
#include "core/common/logger.h"
#include "core/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

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

QVector<Pipeline> PipelineDAO::findAll(int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":limit"] = limit;

    qDebug() << "[PipelineDAO] findAll called, limit:" << limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        qDebug() << "[PipelineDAO] Query error:" << query.lastError().text();
        LOG_ERROR(QString("Pipeline query failed: %1").arg(query.lastError().text()));
    }
    
    while (query.next()) {
        results.append(fromQuery(query));
    }

    qDebug() << "[PipelineDAO] Found" << results.size() << "pipelines";
    LOG_INFO(QString("Found %1 pipelines").arg(results.size()));
    return results;
}

QVector<Pipeline> PipelineDAO::findByType(const QString &type, int limit)
{
    QString sql = QString("SELECT *, ST_AsText(geom) as geom_text "
                          "FROM %1 WHERE pipeline_type = :type LIMIT :limit")
                      .arg(m_tableName);

    QVariantMap params;
    params[":type"] = type;
    params[":limit"] = limit;

    qDebug() << "[PipelineDAO] findByType called:";
    qDebug() << "  Type:" << type << "Limit:" << limit;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        qDebug() << "[PipelineDAO] Query error:" << query.lastError().text();
        LOG_ERROR(QString("Pipeline query failed: %1").arg(query.lastError().text()));
    }
    
    while (query.next()) {
        results.append(fromQuery(query));
    }

    qDebug() << "[PipelineDAO] Found" << results.size() << "pipelines of type" << type;
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

    // 调试输出
    qDebug() << "[PipelineDAO] findByBounds called:";
    qDebug() << "  Input bounds:" << bounds;
    qDebug() << "  minX:" << bounds.left() << "minY:" << bounds.bottom();
    qDebug() << "  maxX:" << bounds.right() << "maxY:" << bounds.top();
    qDebug() << "  SQL:" << sql;

    QVector<Pipeline> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        qDebug() << "[PipelineDAO] Query error:" << query.lastError().text();
        LOG_ERROR(QString("Pipeline query failed: %1").arg(query.lastError().text()));
    }
    
    while (query.next()) {
        results.append(fromQuery(query));
    }

    qDebug() << "[PipelineDAO] Found" << results.size() << "pipelines in bounds";
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

// 转义SQL字符串值，防止SQL注入
static QString escapeSqlString(const QString &str)
{
    QString escaped = str;
    escaped.replace("'", "''");
    return escaped;
}

// 格式化SQL值
static QString formatSqlValue(const QVariant &value)
{
    if (value.isNull() || !value.isValid()) {
        return "NULL";
    }
    
    switch (value.type()) {
        case QVariant::String:
            return QString("'%1'").arg(escapeSqlString(value.toString()));
        case QVariant::Int:
        case QVariant::LongLong:
            return value.toString();
        case QVariant::Double:
            return QString::number(value.toDouble(), 'f', 10);
        case QVariant::Date:
            return QString("'%1'").arg(value.toDate().toString("yyyy-MM-dd"));
        default:
            return QString("'%1'").arg(escapeSqlString(value.toString()));
    }
}

bool PipelineDAO::insert(const Pipeline &pipeline)
{
    // 验证必需字段
    if (pipeline.pipelineId().isEmpty()) {
        qDebug() << "[PipelineDAO] Insert failed: pipeline_id is empty";
        return false;
    }
    if (pipeline.pipelineName().isEmpty()) {
        qDebug() << "[PipelineDAO] Insert failed: pipeline_name is empty";
        return false;
    }
    if (pipeline.pipelineType().isEmpty()) {
        qDebug() << "[PipelineDAO] Insert failed: pipeline_type is empty";
        return false;
    }
    if (pipeline.coordinates().isEmpty()) {
        qDebug() << "[PipelineDAO] Insert failed: coordinates is empty";
        return false;
    }
    
    QVariantMap data = toVariantMap(pipeline);
    
    // 完全手动构建SQL，避免PostGIS函数与参数绑定冲突
    QStringList columns;
    QStringList values;
    
    // 必需字段
    columns.append("pipeline_id");
    values.append(formatSqlValue(data.value("pipeline_id")));
    
    columns.append("pipeline_name");
    values.append(formatSqlValue(data.value("pipeline_name")));
    
    columns.append("pipeline_type");
    values.append(formatSqlValue(data.value("pipeline_type")));
    
    // 几何字段（必需）
    QString geomWkt = data.value("geom_wkt").toString();
    if (geomWkt.isEmpty()) {
        qDebug() << "[PipelineDAO] Insert failed: geom_wkt is empty";
        return false;
    }
    QString escaped = geomWkt;
    escaped.replace("'", "''");
    columns.append("geom");
    values.append(QString("ST_GeomFromText('%1', 4326)").arg(escaped));
    
    // 可选字段
    if (data.value("length_m").toDouble() > 0) {
        columns.append("length_m");
        values.append(formatSqlValue(data.value("length_m")));
    }
    
    if (data.value("depth_m").toDouble() > 0) {
        columns.append("depth_m");
        values.append(formatSqlValue(data.value("depth_m")));
    }
    
    if (data.value("diameter_mm").toInt() > 0) {
        columns.append("diameter_mm");
        values.append(formatSqlValue(data.value("diameter_mm")));
    }
    
    if (!data.value("material").toString().isEmpty()) {
        columns.append("material");
        values.append(formatSqlValue(data.value("material")));
    }
    
    if (!data.value("pressure_class").toString().isEmpty()) {
        columns.append("pressure_class");
        values.append(formatSqlValue(data.value("pressure_class")));
    }
    
    if (data.value("build_date").toDate().isValid() && data.value("build_date").toDate().year() > 2000) {
        columns.append("build_date");
        values.append(formatSqlValue(data.value("build_date")));
    }
    
    if (!data.value("builder").toString().isEmpty()) {
        columns.append("builder");
        values.append(formatSqlValue(data.value("builder")));
    }
    
    if (!data.value("owner").toString().isEmpty()) {
        columns.append("owner");
        values.append(formatSqlValue(data.value("owner")));
    }
    
    if (data.value("construction_cost").toDouble() > 0) {
        columns.append("construction_cost");
        values.append(formatSqlValue(data.value("construction_cost")));
    }
    
    if (!data.value("status").toString().isEmpty()) {
        columns.append("status");
        values.append(formatSqlValue(data.value("status")));
    }
    
    columns.append("health_score");
    values.append(formatSqlValue(data.value("health_score")));
    
    if (data.value("last_inspection").toDate().isValid() && data.value("last_inspection").toDate().year() > 2000) {
        columns.append("last_inspection");
        values.append(formatSqlValue(data.value("last_inspection")));
    }
    
    if (!data.value("maintenance_unit").toString().isEmpty()) {
        columns.append("maintenance_unit");
        values.append(formatSqlValue(data.value("maintenance_unit")));
    }
    
    if (data.value("inspection_cycle").toInt() > 0) {
        columns.append("inspection_cycle");
        values.append(formatSqlValue(data.value("inspection_cycle")));
    }
    
    if (!data.value("remarks").toString().isEmpty()) {
        columns.append("remarks");
        values.append(formatSqlValue(data.value("remarks")));
    }
    
    if (!data.value("created_by").toString().isEmpty()) {
        columns.append("created_by");
        values.append(formatSqlValue(data.value("created_by")));
    }
    
    if (!data.value("updated_by").toString().isEmpty()) {
        columns.append("updated_by");
        values.append(formatSqlValue(data.value("updated_by")));
    }
    
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(m_tableName)
                      .arg(columns.join(", "))
                      .arg(values.join(", "));
    
    qDebug() << "[PipelineDAO] Insert SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[PipelineDAO] Insert failed:" << error;
    }
    return result;
}

bool PipelineDAO::update(const Pipeline &pipeline, int id)
{
    if (id <= 0) {
        qDebug() << "[PipelineDAO] Update failed: invalid pipeline ID";
        return false;
    }
    
    QVariantMap data = toVariantMap(pipeline);
    
    // 完全手动构建SQL，避免PostGIS函数与参数绑定冲突
    QStringList setParts;
    
    // 更新字段
    if (!data.value("pipeline_name").toString().isEmpty()) {
        setParts.append(QString("pipeline_name = '%1'").arg(escapeSqlString(data.value("pipeline_name").toString())));
    }
    
    if (!data.value("pipeline_type").toString().isEmpty()) {
        setParts.append(QString("pipeline_type = '%1'").arg(escapeSqlString(data.value("pipeline_type").toString())));
    }
    
    if (data.value("length_m").toDouble() > 0) {
        setParts.append(QString("length_m = %1").arg(data.value("length_m").toDouble(), 0, 'f', 2));
    }
    
    if (data.value("depth_m").toDouble() > 0) {
        setParts.append(QString("depth_m = %1").arg(data.value("depth_m").toDouble(), 0, 'f', 2));
    }
    
    if (data.value("diameter_mm").toInt() > 0) {
        setParts.append(QString("diameter_mm = %1").arg(data.value("diameter_mm").toInt()));
    }
    
    if (!data.value("material").toString().isEmpty()) {
        setParts.append(QString("material = '%1'").arg(escapeSqlString(data.value("material").toString())));
    }
    
    if (!data.value("pressure_class").toString().isEmpty()) {
        setParts.append(QString("pressure_class = '%1'").arg(escapeSqlString(data.value("pressure_class").toString())));
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
    
    if (data.value("construction_cost").toDouble() > 0) {
        setParts.append(QString("construction_cost = %1").arg(data.value("construction_cost").toDouble(), 0, 'f', 2));
    }
    
    if (!data.value("status").toString().isEmpty()) {
        setParts.append(QString("status = '%1'").arg(escapeSqlString(data.value("status").toString())));
    }
    
    setParts.append(QString("health_score = %1").arg(data.value("health_score").toInt()));
    
    if (data.value("last_inspection").toDate().isValid() && data.value("last_inspection").toDate().year() > 2000) {
        setParts.append(QString("last_inspection = '%1'").arg(data.value("last_inspection").toDate().toString("yyyy-MM-dd")));
    }
    
    if (!data.value("maintenance_unit").toString().isEmpty()) {
        setParts.append(QString("maintenance_unit = '%1'").arg(escapeSqlString(data.value("maintenance_unit").toString())));
    }
    
    if (data.value("inspection_cycle").toInt() > 0) {
        setParts.append(QString("inspection_cycle = %1").arg(data.value("inspection_cycle").toInt()));
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
        qDebug() << "[PipelineDAO] No fields to update";
        return false;
    }
    
    QString sql = QString("UPDATE %1 SET %2 WHERE id = %3")
                      .arg(m_tableName)
                      .arg(setParts.join(", "))
                      .arg(id);
    
    qDebug() << "[PipelineDAO] Update SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[PipelineDAO] Update failed:" << error;
    }
    return result;
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

