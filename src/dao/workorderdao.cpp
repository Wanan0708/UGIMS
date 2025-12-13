#include "workorderdao.h"
#include "core/common/logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QRegularExpression>

WorkOrderDAO::WorkOrderDAO()
    : BaseDAO<WorkOrder>("work_orders")
{
    // 检查并创建表（如果不存在）
    ensureTableExists();
}

void WorkOrderDAO::ensureTableExists()
{
    // 检查表是否存在
    QString checkSql = QString(
        "SELECT EXISTS ("
        "  SELECT FROM information_schema.tables "
        "  WHERE table_schema = 'public' "
        "  AND table_name = '%1'"
        ")"
    ).arg(m_tableName);
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(checkSql);
    bool tableExists = false;
    if (query.next()) {
        tableExists = query.value(0).toBool();
    }
    
    if (!tableExists) {
        qDebug() << "[WorkOrderDAO] Table does not exist, creating...";
        createTable();
    }
}

void WorkOrderDAO::createTable()
{
    QString createSql = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "  id SERIAL PRIMARY KEY,"
        "  order_id VARCHAR(50) UNIQUE NOT NULL,"
        "  order_title VARCHAR(200) NOT NULL,"
        "  order_type VARCHAR(50) NOT NULL,"
        "  priority VARCHAR(20) DEFAULT 'normal',"
        "  asset_type VARCHAR(20),"
        "  asset_id VARCHAR(50),"
        "  location GEOMETRY(POINT, 4326),"
        "  description TEXT,"
        "  required_actions TEXT,"
        "  assigned_to VARCHAR(100),"
        "  assigned_at TIMESTAMP,"
        "  assigned_by VARCHAR(100),"
        "  status VARCHAR(50) DEFAULT 'pending',"
        "  plan_start_time TIMESTAMP,"
        "  plan_end_time TIMESTAMP,"
        "  actual_start_time TIMESTAMP,"
        "  actual_end_time TIMESTAMP,"
        "  completion_rate INTEGER DEFAULT 0 CHECK (completion_rate >= 0 AND completion_rate <= 100),"
        "  work_result TEXT,"
        "  photos TEXT[],"
        "  reviewed_by VARCHAR(100),"
        "  reviewed_at TIMESTAMP,"
        "  review_result VARCHAR(20),"
        "  review_comments TEXT,"
        "  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "  created_by VARCHAR(100)"
        ")"
    ).arg(m_tableName);
    
    if (!DatabaseManager::instance().executeCommand(createSql)) {
        qDebug() << "[WorkOrderDAO] Failed to create table:" << DatabaseManager::instance().lastError();
        return;
    }
    
    // 创建索引
    QString indexSql1 = QString("CREATE INDEX IF NOT EXISTS idx_work_orders_type ON %1(order_type)").arg(m_tableName);
    QString indexSql2 = QString("CREATE INDEX IF NOT EXISTS idx_work_orders_status ON %1(status)").arg(m_tableName);
    QString indexSql3 = QString("CREATE INDEX IF NOT EXISTS idx_work_orders_assigned ON %1(assigned_to)").arg(m_tableName);
    QString indexSql4 = QString("CREATE INDEX IF NOT EXISTS idx_work_orders_created ON %1(created_at)").arg(m_tableName);
    
    DatabaseManager::instance().executeCommand(indexSql1);
    DatabaseManager::instance().executeCommand(indexSql2);
    DatabaseManager::instance().executeCommand(indexSql3);
    DatabaseManager::instance().executeCommand(indexSql4);
    
    // 创建空间索引（如果PostGIS可用）
    QString spatialIndexSql = QString("CREATE INDEX IF NOT EXISTS idx_work_orders_location ON %1 USING GIST(location)").arg(m_tableName);
    DatabaseManager::instance().executeCommand(spatialIndexSql);
    
    qDebug() << "[WorkOrderDAO] Table created successfully";
}

WorkOrder WorkOrderDAO::fromQuery(QSqlQuery &query)
{
    WorkOrder workOrder;
    
    workOrder.setId(query.value("id").toInt());
    workOrder.setOrderId(query.value("order_id").toString());
    workOrder.setOrderTitle(query.value("order_title").toString());
    workOrder.setOrderType(query.value("order_type").toString());
    workOrder.setPriority(query.value("priority").toString());
    
    // 关联资产
    workOrder.setAssetType(query.value("asset_type").toString());
    workOrder.setAssetId(query.value("asset_id").toString());
    
    // 解析位置（PostGIS POINT）
    QString locationWkt = query.value("location_text").toString();
    if (!locationWkt.isEmpty()) {
        workOrder.setLocation(parsePointWkt(locationWkt));
    }
    
    // 工单内容
    workOrder.setDescription(query.value("description").toString());
    workOrder.setRequiredActions(query.value("required_actions").toString());
    
    // 人员分配
    workOrder.setAssignedTo(query.value("assigned_to").toString());
    workOrder.setAssignedAt(query.value("assigned_at").toDateTime());
    workOrder.setAssignedBy(query.value("assigned_by").toString());
    
    // 状态
    workOrder.setStatus(query.value("status").toString());
    
    // 时间节点
    workOrder.setPlanStartTime(query.value("plan_start_time").toDateTime());
    workOrder.setPlanEndTime(query.value("plan_end_time").toDateTime());
    workOrder.setActualStartTime(query.value("actual_start_time").toDateTime());
    workOrder.setActualEndTime(query.value("actual_end_time").toDateTime());
    
    // 完成情况
    workOrder.setCompletionRate(query.value("completion_rate").toInt());
    workOrder.setWorkResult(query.value("work_result").toString());
    
    // 解析照片数组
    QString photosStr = query.value("photos_text").toString();
    if (!photosStr.isEmpty()) {
        workOrder.setPhotos(parseArray(photosStr));
    }
    
    // 审核
    workOrder.setReviewedBy(query.value("reviewed_by").toString());
    workOrder.setReviewedAt(query.value("reviewed_at").toDateTime());
    workOrder.setReviewResult(query.value("review_result").toString());
    workOrder.setReviewComments(query.value("review_comments").toString());
    
    // 元数据
    workOrder.setCreatedAt(query.value("created_at").toDateTime());
    workOrder.setUpdatedAt(query.value("updated_at").toDateTime());
    workOrder.setCreatedBy(query.value("created_by").toString());
    
    return workOrder;
}

QVariantMap WorkOrderDAO::toVariantMap(const WorkOrder &workOrder)
{
    QVariantMap data;
    
    if (workOrder.id() > 0) {
        data["id"] = workOrder.id();
    }
    
    data["order_id"] = workOrder.orderId();
    data["order_title"] = workOrder.orderTitle();
    data["order_type"] = workOrder.orderType();
    data["priority"] = workOrder.priority();
    
    data["asset_type"] = workOrder.assetType();
    data["asset_id"] = workOrder.assetId();
    
    // 位置转换为PostGIS POINT（需要在SQL中处理，这里只存储坐标）
    if (!workOrder.location().isNull()) {
        // 标记为需要特殊处理的字段
        data["location_x"] = workOrder.location().x();
        data["location_y"] = workOrder.location().y();
        data["has_location"] = true;
    } else {
        data["has_location"] = false;
    }
    
    data["description"] = workOrder.description();
    data["required_actions"] = workOrder.requiredActions();
    
    data["assigned_to"] = workOrder.assignedTo();
    if (workOrder.assignedAt().isValid()) {
        data["assigned_at"] = workOrder.assignedAt();
    }
    data["assigned_by"] = workOrder.assignedBy();
    
    data["status"] = workOrder.status();
    
    if (workOrder.planStartTime().isValid()) {
        data["plan_start_time"] = workOrder.planStartTime();
    }
    if (workOrder.planEndTime().isValid()) {
        data["plan_end_time"] = workOrder.planEndTime();
    }
    if (workOrder.actualStartTime().isValid()) {
        data["actual_start_time"] = workOrder.actualStartTime();
    }
    if (workOrder.actualEndTime().isValid()) {
        data["actual_end_time"] = workOrder.actualEndTime();
    }
    
    data["completion_rate"] = workOrder.completionRate();
    data["work_result"] = workOrder.workResult();
    
    // 照片数组
    if (!workOrder.photos().isEmpty()) {
        data["photos"] = toArrayString(workOrder.photos());
    }
    
    data["reviewed_by"] = workOrder.reviewedBy();
    if (workOrder.reviewedAt().isValid()) {
        data["reviewed_at"] = workOrder.reviewedAt();
    }
    data["review_result"] = workOrder.reviewResult();
    data["review_comments"] = workOrder.reviewComments();
    
    if (workOrder.createdAt().isValid()) {
        data["created_at"] = workOrder.createdAt();
    }
    if (workOrder.updatedAt().isValid()) {
        data["updated_at"] = workOrder.updatedAt();
    }
    data["created_by"] = workOrder.createdBy();
    
    return data;
}

WorkOrder WorkOrderDAO::findByOrderId(const QString &orderId)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE order_id = :order_id")
                      .arg(m_tableName);
    
    QVariantMap params;
    params[":order_id"] = orderId;
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    if (query.next()) {
        return fromQuery(query);
    }
    return WorkOrder();
}

QVector<WorkOrder> WorkOrderDAO::findByStatus(const QString &status, int limit)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE status = :status ORDER BY created_at DESC LIMIT %2")
                      .arg(m_tableName)
                      .arg(limit);
    
    QVariantMap params;
    params[":status"] = status;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<WorkOrder> WorkOrderDAO::findByType(const QString &type, int limit)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE order_type = :type ORDER BY created_at DESC LIMIT %2")
                      .arg(m_tableName)
                      .arg(limit);
    
    QVariantMap params;
    params[":type"] = type;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<WorkOrder> WorkOrderDAO::findByPriority(const QString &priority, int limit)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE priority = :priority ORDER BY created_at DESC LIMIT %2")
                      .arg(m_tableName)
                      .arg(limit);
    
    QVariantMap params;
    params[":priority"] = priority;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<WorkOrder> WorkOrderDAO::findByAssignedTo(const QString &assignedTo, int limit)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE assigned_to = :assigned_to ORDER BY created_at DESC LIMIT %2")
                      .arg(m_tableName)
                      .arg(limit);
    
    QVariantMap params;
    params[":assigned_to"] = assignedTo;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<WorkOrder> WorkOrderDAO::findByDateRange(const QDateTime &startDate, const QDateTime &endDate, int limit)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 WHERE created_at >= :start_date AND created_at <= :end_date "
                          "ORDER BY created_at DESC LIMIT %2")
                      .arg(m_tableName)
                      .arg(limit);
    
    QVariantMap params;
    params[":start_date"] = startDate;
    params[":end_date"] = endDate;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QVector<WorkOrder> WorkOrderDAO::findAll(int limit, int offset)
{
    QString sql = QString("SELECT *, "
                          "CASE WHEN location IS NOT NULL THEN ST_AsText(location) ELSE NULL END as location_text, "
                          "CASE WHEN photos IS NOT NULL THEN array_to_string(photos, ',') ELSE NULL END as photos_text "
                          "FROM %1 ORDER BY created_at DESC LIMIT %2 OFFSET %3")
                      .arg(m_tableName)
                      .arg(limit)
                      .arg(offset);
    
    QVariantMap params;
    
    QVector<WorkOrder> results;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql, params);
    while (query.next()) {
        results.append(fromQuery(query));
    }
    return results;
}

QMap<QString, int> WorkOrderDAO::countByStatus()
{
    QString sql = QString("SELECT status, COUNT(*) as count FROM %1 GROUP BY status")
                      .arg(m_tableName);
    
    QMap<QString, int> counts;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    while (query.next()) {
        QString status = query.value("status").toString();
        int count = query.value("count").toInt();
        counts[status] = count;
    }
    return counts;
}

QMap<QString, int> WorkOrderDAO::countByType()
{
    QString sql = QString("SELECT order_type, COUNT(*) as count FROM %1 GROUP BY order_type")
                      .arg(m_tableName);
    
    QMap<QString, int> counts;
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    while (query.next()) {
        QString type = query.value("order_type").toString();
        int count = query.value("count").toInt();
        counts[type] = count;
    }
    return counts;
}

QString WorkOrderDAO::generateOrderId()
{
    // 生成格式: WO20250126001 (WO + 日期 + 序号)
    QDate today = QDate::currentDate();
    QString dateStr = today.toString("yyyyMMdd");
    
    // 查找今天已有的工单数量
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE order_id LIKE 'WO%2%'")
                      .arg(m_tableName)
                      .arg(dateStr);
    
    QSqlQuery query = DatabaseManager::instance().executeQuery(sql);
    int count = 0;
    if (query.next()) {
        count = query.value(0).toInt();
    }
    
    // 生成序号（3位数字，从001开始）
    QString sequence = QString("%1").arg(count + 1, 3, 10, QChar('0'));
    
    return QString("WO%1%2").arg(dateStr).arg(sequence);
}

QPointF WorkOrderDAO::parsePointWkt(const QString &wkt)
{
    // 解析格式: POINT(116.404 39.915)
    QRegularExpression re(R"(POINT\(([\d.]+)\s+([\d.]+)\))");
    QRegularExpressionMatch match = re.match(wkt);
    
    if (match.hasMatch()) {
        double lon = match.captured(1).toDouble();
        double lat = match.captured(2).toDouble();
        return QPointF(lon, lat);
    }
    
    return QPointF();
}

QString WorkOrderDAO::toPointWkt(const QPointF &point)
{
    return QString("POINT(%1 %2)").arg(point.x()).arg(point.y());
}

QStringList WorkOrderDAO::parseArray(const QString &arrayStr)
{
    // 解析PostgreSQL数组格式: {item1,item2,item3}
    QStringList result;
    if (arrayStr.isEmpty() || arrayStr == "{}") {
        return result;
    }
    
    QString cleaned = arrayStr;
    cleaned.remove('{');
    cleaned.remove('}');
    
    result = cleaned.split(',', Qt::SkipEmptyParts);
    return result;
}

QString WorkOrderDAO::toArrayString(const QStringList &list)
{
    if (list.isEmpty()) {
        return "{}";
    }
    
    QString result = "{";
    for (int i = 0; i < list.size(); ++i) {
        if (i > 0) result += ",";
        result += list[i];
    }
    result += "}";
    return result;
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
        case QVariant::DateTime:
            return QString("'%1'").arg(value.toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        case QVariant::Date:
            return QString("'%1'").arg(value.toDate().toString("yyyy-MM-dd"));
        default:
            return QString("'%1'").arg(escapeSqlString(value.toString()));
    }
}

bool WorkOrderDAO::deleteByOrderId(const QString &orderId)
{
    QString sql = QString("DELETE FROM %1 WHERE order_id = '%2'")
                      .arg(m_tableName)
                      .arg(escapeSqlString(orderId));
    return DatabaseManager::instance().executeCommand(sql);
}

bool WorkOrderDAO::insert(const WorkOrder &workOrder)
{
    // 验证必需字段
    if (workOrder.orderId().isEmpty()) {
        qDebug() << "[WorkOrderDAO] Insert failed: order_id is empty";
        return false;
    }
    if (workOrder.orderTitle().isEmpty()) {
        qDebug() << "[WorkOrderDAO] Insert failed: order_title is empty";
        return false;
    }
    if (workOrder.orderType().isEmpty()) {
        qDebug() << "[WorkOrderDAO] Insert failed: order_type is empty";
        return false;
    }
    
    QVariantMap data = toVariantMap(workOrder);
    
    // 由于PostGIS函数不能与参数绑定混合使用，完全手动构建SQL
    QStringList columns;
    QStringList values;
    
    // 必需字段
    columns.append("order_id");
    values.append(formatSqlValue(data.value("order_id")));
    
    columns.append("order_title");
    values.append(formatSqlValue(data.value("order_title")));
    
    columns.append("order_type");
    values.append(formatSqlValue(data.value("order_type")));
    
    // 可选字段
    if (!data.value("priority").toString().isEmpty()) {
        columns.append("priority");
        values.append(formatSqlValue(data.value("priority")));
    }
    
    if (!data.value("status").toString().isEmpty()) {
        columns.append("status");
        values.append(formatSqlValue(data.value("status")));
    }
    
    if (!data.value("asset_type").toString().isEmpty()) {
        columns.append("asset_type");
        values.append(formatSqlValue(data.value("asset_type")));
    }
    
    if (!data.value("asset_id").toString().isEmpty()) {
        columns.append("asset_id");
        values.append(formatSqlValue(data.value("asset_id")));
    }
    
    // 位置字段
    if (data.value("has_location").toBool()) {
        double lon = data.value("location_x").toDouble();
        double lat = data.value("location_y").toDouble();
        columns.append("location");
        values.append(QString("ST_SetSRID(ST_MakePoint(%1, %2), 4326)")
                     .arg(lon, 0, 'f', 10)
                     .arg(lat, 0, 'f', 10));
    }
    
    if (!data.value("description").toString().isEmpty()) {
        columns.append("description");
        values.append(formatSqlValue(data.value("description")));
    }
    
    if (!data.value("required_actions").toString().isEmpty()) {
        columns.append("required_actions");
        values.append(formatSqlValue(data.value("required_actions")));
    }
    
    if (!data.value("assigned_to").toString().isEmpty()) {
        columns.append("assigned_to");
        values.append(formatSqlValue(data.value("assigned_to")));
    }
    
    if (data.value("assigned_at").toDateTime().isValid()) {
        columns.append("assigned_at");
        values.append(formatSqlValue(data.value("assigned_at")));
    }
    
    if (!data.value("assigned_by").toString().isEmpty()) {
        columns.append("assigned_by");
        values.append(formatSqlValue(data.value("assigned_by")));
    }
    
    QDateTime planStart = data.value("plan_start_time").toDateTime();
    if (planStart.isValid() && planStart.date().year() > 2000) {
        columns.append("plan_start_time");
        values.append(formatSqlValue(planStart));
    }
    
    QDateTime planEnd = data.value("plan_end_time").toDateTime();
    if (planEnd.isValid() && planEnd.date().year() > 2000) {
        columns.append("plan_end_time");
        values.append(formatSqlValue(planEnd));
    }
    
    QDateTime actualStart = data.value("actual_start_time").toDateTime();
    if (actualStart.isValid() && actualStart.date().year() > 2000) {
        columns.append("actual_start_time");
        values.append(formatSqlValue(actualStart));
    }
    
    QDateTime actualEnd = data.value("actual_end_time").toDateTime();
    if (actualEnd.isValid() && actualEnd.date().year() > 2000) {
        columns.append("actual_end_time");
        values.append(formatSqlValue(actualEnd));
    }
    
    if (data.value("completion_rate").toInt() > 0) {
        columns.append("completion_rate");
        values.append(formatSqlValue(data.value("completion_rate")));
    }
    
    if (!data.value("work_result").toString().isEmpty()) {
        columns.append("work_result");
        values.append(formatSqlValue(data.value("work_result")));
    }
    
    // 照片数组
    QString photosArrayStr = data.value("photos").toString();
    if (!photosArrayStr.isEmpty()) {
        columns.append("photos");
        QString escaped = photosArrayStr;
        escaped.replace("'", "''");
        values.append(QString("'%1'::text[]").arg(escaped));
    }
    
    if (!data.value("reviewed_by").toString().isEmpty()) {
        columns.append("reviewed_by");
        values.append(formatSqlValue(data.value("reviewed_by")));
    }
    
    if (data.value("reviewed_at").toDateTime().isValid()) {
        columns.append("reviewed_at");
        values.append(formatSqlValue(data.value("reviewed_at")));
    }
    
    if (!data.value("review_result").toString().isEmpty()) {
        columns.append("review_result");
        values.append(formatSqlValue(data.value("review_result")));
    }
    
    if (!data.value("review_comments").toString().isEmpty()) {
        columns.append("review_comments");
        values.append(formatSqlValue(data.value("review_comments")));
    }
    
    if (!data.value("created_by").toString().isEmpty()) {
        columns.append("created_by");
        values.append(formatSqlValue(data.value("created_by")));
    }
    
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(m_tableName)
                      .arg(columns.join(", "))
                      .arg(values.join(", "));
    
    qDebug() << "[WorkOrderDAO] Insert SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[WorkOrderDAO] Insert failed:" << error;
    }
    return result;
}

bool WorkOrderDAO::update(const WorkOrder &workOrder)
{
    if (workOrder.id() <= 0) {
        qDebug() << "[WorkOrderDAO] Update failed: invalid work order ID";
        return false;
    }
    
    QVariantMap data = toVariantMap(workOrder);
    
    // 完全手动构建SQL，避免PostGIS函数与参数绑定冲突
    QStringList setParts;
    
    // 更新字段
    if (!data.value("order_title").toString().isEmpty()) {
        setParts.append(QString("order_title = %1").arg(formatSqlValue(data.value("order_title"))));
    }
    
    if (!data.value("order_type").toString().isEmpty()) {
        setParts.append(QString("order_type = %1").arg(formatSqlValue(data.value("order_type"))));
    }
    
    if (!data.value("priority").toString().isEmpty()) {
        setParts.append(QString("priority = %1").arg(formatSqlValue(data.value("priority"))));
    }
    
    if (!data.value("status").toString().isEmpty()) {
        setParts.append(QString("status = %1").arg(formatSqlValue(data.value("status"))));
    }
    
    if (!data.value("asset_type").toString().isEmpty()) {
        setParts.append(QString("asset_type = %1").arg(formatSqlValue(data.value("asset_type"))));
    } else {
        setParts.append("asset_type = NULL");
    }
    
    if (!data.value("asset_id").toString().isEmpty()) {
        setParts.append(QString("asset_id = %1").arg(formatSqlValue(data.value("asset_id"))));
    } else {
        setParts.append("asset_id = NULL");
    }
    
    // 位置字段
    if (data.value("has_location").toBool()) {
        double lon = data.value("location_x").toDouble();
        double lat = data.value("location_y").toDouble();
        setParts.append(QString("location = ST_SetSRID(ST_MakePoint(%1, %2), 4326)")
                       .arg(lon, 0, 'f', 10)
                       .arg(lat, 0, 'f', 10));
    } else {
        setParts.append("location = NULL");
    }
    
    if (!data.value("description").toString().isEmpty()) {
        setParts.append(QString("description = %1").arg(formatSqlValue(data.value("description"))));
    } else {
        setParts.append("description = NULL");
    }
    
    if (!data.value("required_actions").toString().isEmpty()) {
        setParts.append(QString("required_actions = %1").arg(formatSqlValue(data.value("required_actions"))));
    } else {
        setParts.append("required_actions = NULL");
    }
    
    if (!data.value("assigned_to").toString().isEmpty()) {
        setParts.append(QString("assigned_to = %1").arg(formatSqlValue(data.value("assigned_to"))));
    } else {
        setParts.append("assigned_to = NULL");
    }
    
    if (data.value("assigned_at").toDateTime().isValid()) {
        setParts.append(QString("assigned_at = %1").arg(formatSqlValue(data.value("assigned_at"))));
    } else {
        setParts.append("assigned_at = NULL");
    }
    
    if (!data.value("assigned_by").toString().isEmpty()) {
        setParts.append(QString("assigned_by = %1").arg(formatSqlValue(data.value("assigned_by"))));
    } else {
        setParts.append("assigned_by = NULL");
    }
    
    if (data.value("plan_start_time").toDateTime().isValid()) {
        setParts.append(QString("plan_start_time = %1").arg(formatSqlValue(data.value("plan_start_time"))));
    } else {
        setParts.append("plan_start_time = NULL");
    }
    
    if (data.value("plan_end_time").toDateTime().isValid()) {
        setParts.append(QString("plan_end_time = %1").arg(formatSqlValue(data.value("plan_end_time"))));
    } else {
        setParts.append("plan_end_time = NULL");
    }
    
    if (data.value("actual_start_time").toDateTime().isValid()) {
        setParts.append(QString("actual_start_time = %1").arg(formatSqlValue(data.value("actual_start_time"))));
    } else {
        setParts.append("actual_start_time = NULL");
    }
    
    if (data.value("actual_end_time").toDateTime().isValid()) {
        setParts.append(QString("actual_end_time = %1").arg(formatSqlValue(data.value("actual_end_time"))));
    } else {
        setParts.append("actual_end_time = NULL");
    }
    
    setParts.append(QString("completion_rate = %1").arg(formatSqlValue(data.value("completion_rate"))));
    
    if (!data.value("work_result").toString().isEmpty()) {
        setParts.append(QString("work_result = %1").arg(formatSqlValue(data.value("work_result"))));
    } else {
        setParts.append("work_result = NULL");
    }
    
    // 照片数组
    QString photosArrayStr = data.value("photos").toString();
    if (!photosArrayStr.isEmpty()) {
        QString escaped = photosArrayStr;
        escaped.replace("'", "''");
        setParts.append(QString("photos = '%1'::text[]").arg(escaped));
    } else {
        setParts.append("photos = NULL");
    }
    
    if (!data.value("reviewed_by").toString().isEmpty()) {
        setParts.append(QString("reviewed_by = %1").arg(formatSqlValue(data.value("reviewed_by"))));
    } else {
        setParts.append("reviewed_by = NULL");
    }
    
    if (data.value("reviewed_at").toDateTime().isValid()) {
        setParts.append(QString("reviewed_at = %1").arg(formatSqlValue(data.value("reviewed_at"))));
    } else {
        setParts.append("reviewed_at = NULL");
    }
    
    if (!data.value("review_result").toString().isEmpty()) {
        setParts.append(QString("review_result = %1").arg(formatSqlValue(data.value("review_result"))));
    } else {
        setParts.append("review_result = NULL");
    }
    
    if (!data.value("review_comments").toString().isEmpty()) {
        setParts.append(QString("review_comments = %1").arg(formatSqlValue(data.value("review_comments"))));
    } else {
        setParts.append("review_comments = NULL");
    }
    
    setParts.append("updated_at = CURRENT_TIMESTAMP");
    
    if (setParts.isEmpty()) {
        qDebug() << "[WorkOrderDAO] No fields to update";
        return false;
    }
    
    QString sql = QString("UPDATE %1 SET %2 WHERE id = %3")
                      .arg(m_tableName)
                      .arg(setParts.join(", "))
                      .arg(workOrder.id());
    
    qDebug() << "[WorkOrderDAO] Update SQL:" << sql;
    
    // 直接执行SQL，不使用参数绑定
    bool result = DatabaseManager::instance().executeCommand(sql);
    if (!result) {
        QString error = DatabaseManager::instance().lastError();
        qDebug() << "[WorkOrderDAO] Update failed:" << error;
    }
    return result;
}

