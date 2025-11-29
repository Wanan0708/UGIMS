#include "burstanalyzer.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/common/logger.h"
#include <QDebug>
#include <QtMath>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QQueue>
#include <QSet>

BurstAnalyzer::BurstAnalyzer(QObject *parent)
    : SpatialAnalyzer(parent)
    , m_searchRadius(500.0)      // 默认500米搜索半径
    , m_maxValveDistance(1000.0) // 默认1000米最大阀门距离
    , m_userDensity(5000)        // 默认5000户/km²
{
}

BurstAnalyzer::~BurstAnalyzer()
{
}

void BurstAnalyzer::setSearchRadius(double radius)
{
    m_searchRadius = radius;
}

void BurstAnalyzer::setMaxValveDistance(double distance)
{
    m_maxValveDistance = distance;
}

void BurstAnalyzer::setUserDensity(int usersPerKm2)
{
    m_userDensity = usersPerKm2;
}

BurstAnalysisResult BurstAnalyzer::analyzeBurst(const QPointF &burstPoint, const QString &pipelineId)
{
    qDebug() << "[BurstAnalyzer] Starting burst analysis at point:" << burstPoint;
    
    BurstAnalysisResult result;
    result.burstLocation = burstPoint;
    
    try {
        // Step 1: 如果未指定管线ID，查找最近的管线
        QString targetPipelineId = pipelineId;
        if (targetPipelineId.isEmpty()) {
            emit analysisProgress(10, 100, "正在查找爆管管线...");
            targetPipelineId = findNearestPipeline(burstPoint);
            
            if (targetPipelineId.isEmpty()) {
                result.success = false;
                result.message = "未找到附近的管线";
                return result;
            }
        }
        
        result.pipelineId = targetPipelineId;
        qDebug() << "[BurstAnalyzer] Target pipeline:" << targetPipelineId;
        
        // Step 2: 查找需要关闭的阀门
        emit analysisProgress(30, 100, "正在查找相关阀门...");
        result.affectedValves = findAffectedValves(targetPipelineId, burstPoint);
        qDebug() << "[BurstAnalyzer] Found" << result.affectedValves.size() << "affected valves";
        
        // Step 3: 追踪连通的管线
        emit analysisProgress(50, 100, "正在追踪连通管线...");
        result.affectedPipelines = traceConnectedPipelines(targetPipelineId, burstPoint);
        qDebug() << "[BurstAnalyzer] Found" << result.affectedPipelines.size() << "affected pipelines";
        
        // Step 4: 计算影响区域
        emit analysisProgress(70, 100, "正在计算影响区域...");
        result.affectedArea = calculateAffectedArea(result.affectedPipelines);
        result.affectedAreaSize = calculatePolygonArea(result.affectedArea);
        
        // Step 5: 估计受影响用户
        emit analysisProgress(85, 100, "正在估算影响用户...");
        result.estimatedAffectedUsers = estimateAffectedUsers(result.affectedArea);
        
        // Step 6: 计算其他数据
        result.affectedLength = m_searchRadius * 2; // 简化计算
        result.repairPriority = calculateRepairPriority(result.pipelineType, result.pipelineDiameter, result.estimatedAffectedUsers);
        result.estimatedRepairTime = 4.0 + (result.pipelineDiameter / 100.0); // 基于管径估算
        
        // Step 7: 生成维修建议
        emit analysisProgress(95, 100, "正在生成维修建议...");
        result.suggestedActions = generateRepairSuggestions(result);
        
        // Step 8: 添加应急联系人
        result.emergencyContacts = {
            "调度中心: 12345",
            "抢修队: 110-1234-5678",
            "片区负责人: 张工 139-xxxx-xxxx"
        };
        
        result.success = true;
        result.message = "分析完成";
        
        emit analysisProgress(100, 100, "分析完成");
        
    } catch (const std::exception &e) {
        qWarning() << "[BurstAnalyzer] Analysis error:" << e.what();
        result.success = false;
        result.message = QString("分析失败: %1").arg(e.what());
    }
    
    return result;
}

void BurstAnalyzer::analyzeBurstAsync(const QPointF &burstPoint, const QString &pipelineId)
{
    // 使用QtConcurrent异步执行分析
    QFuture<BurstAnalysisResult> future = QtConcurrent::run([this, burstPoint, pipelineId]() {
        return analyzeBurst(burstPoint, pipelineId);
    });
    
    // 监控异步任务完成
    QFutureWatcher<BurstAnalysisResult> *watcher = new QFutureWatcher<BurstAnalysisResult>(this);
    connect(watcher, &QFutureWatcher<BurstAnalysisResult>::finished, this, [this, watcher]() {
        emit burstAnalysisFinished(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

// ========================================
// 私有辅助方法
// ========================================

QString BurstAnalyzer::findNearestPipeline(const QPointF &point)
{
    qDebug() << "[BurstAnalyzer] Finding nearest pipeline to point:" << point;
    
    try {
        // 在以爆管点为中心、500米范围内查找所有管线
        PipelineDAO pipelineDao;
        QVector<Pipeline> nearbyPipelines = pipelineDao.findNearPoint(point.x(), point.y(), 500.0, 100);
        
        if (nearbyPipelines.isEmpty()) {
            LOG_WARNING("No pipelines found near burst point");
            qDebug() << "[BurstAnalyzer] No pipelines found near point";
            return QString();
        }
        
        // 找最近的管线
        QString nearestId = nearbyPipelines.at(0).pipelineId();
        double minDistance = std::numeric_limits<double>::max();
        
        for (const Pipeline &pipeline : nearbyPipelines) {
            // 计算爆管点到管线的最短距离
            double distance = std::numeric_limits<double>::max();
                            
            for (int i = 0; i < pipeline.coordinates().size() - 1; i++) {
                const QPointF &p1 = pipeline.coordinates().at(i);
                const QPointF &p2 = pipeline.coordinates().at(i + 1);
                                
                // 点到线段的距离
                QLineF segment(p1, p2);
                double d = pointToLineDistance(point, segment) * 111000; // 转换为米
                distance = qMin(distance, d);
            }
            
            if (distance < minDistance) {
                minDistance = distance;
                nearestId = pipeline.pipelineId();
            }
        }
        
        LOG_INFO(QString("Found nearest pipeline: %1 (distance: %2m)").arg(nearestId).arg(minDistance));
        qDebug() << "[BurstAnalyzer] Found nearest pipeline:" << nearestId << "distance:" << minDistance << "m";
        
        return nearestId;
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Error finding nearest pipeline: %1").arg(e.what()));
        qWarning() << "[BurstAnalyzer] Error:" << e.what();
        return QString();
    }
}

QList<QString> BurstAnalyzer::findAffectedValves(const QString &pipelineId, const QPointF &burstPoint)
{
    QList<QString> valveList;
    qDebug() << "[BurstAnalyzer] Finding affected valves for pipeline:" << pipelineId << "at burst point:" << burstPoint;
    
    try {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(pipelineId);
        
        if (!pipeline.isValid()) {
            LOG_WARNING(QString("Pipeline not found: %1").arg(pipelineId));
            return valveList;
        }
        
        // 查找附近的所有阀门
        FacilityDAO facilityDao;
        QVector<Facility> allFacilities = facilityDao.findByType("valve", 1000);
        
        // 同时查找其他类型的关键设施（如消火栓、接头）
        QVector<Facility> otherFacilities = facilityDao.findByType("junction", 1000);
        allFacilities.append(otherFacilities);
        
        // 计算管线的上游和下游阀门
        // 首先，找到爆管点在管线上的位置
        int burstSegmentIndex = -1;
        double minDistanceToBurst = std::numeric_limits<double>::max();
        
        for (int i = 0; i < pipeline.coordinates().size() - 1; i++) {
            const QPointF &p1 = pipeline.coordinates().at(i);
            const QPointF &p2 = pipeline.coordinates().at(i + 1);
            
            QLineF segment(p1, p2);
            double d = pointToLineDistance(burstPoint, segment) * 111000; // 转换为米
            if (d < minDistanceToBurst) {
                minDistanceToBurst = d;
                burstSegmentIndex = i;
            }
        }
        
        if (burstSegmentIndex >= 0) {
            // 查找上游阀门（在爆管点之前）
        for (const Facility &facility : allFacilities) {
                if (facility.facilityType() != "valve") continue;
                
                // 检查阀门是否在管线的上游
                QLineF pipelineLine(pipeline.coordinates().at(0),
                                    pipeline.coordinates().at(pipeline.coordinates().size() - 1));
                double distToBurst = pointToLineDistance(facility.coordinate(), pipelineLine) * 111000;
                
                if (distToBurst < m_maxValveDistance) {
                    double distance = std::sqrt(std::pow(facility.coordinate().x() - burstPoint.x(), 2) +
                                              std::pow(facility.coordinate().y() - burstPoint.y(), 2)) * 111000; // 粗略转为米
                    
                    if (distance < m_maxValveDistance) {
                        valveList.append(QString("%1 (%2m, %3)").arg(facility.facilityId())
                                                              .arg(int(distance))
                                                              .arg("需关闭"));
                    }
                }
            }
        }
        
        LOG_INFO(QString("Found %1 affected valves").arg(valveList.size()));
        qDebug() << "[BurstAnalyzer] Found" << valveList.size() << "affected valves";
        
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Error finding affected valves: %1").arg(e.what()));
        qWarning() << "[BurstAnalyzer] Error:" << e.what();
    }
    
    return valveList;
}

QList<QString> BurstAnalyzer::traceConnectedPipelines(const QString &pipelineId, const QPointF &burstPoint)
{
    QList<QString> tracedPipelines;
    qDebug() << "[BurstAnalyzer] Tracing connected pipelines from:" << pipelineId << "at point:" << burstPoint;
    
    try {
        PipelineDAO pipelineDao;
        FacilityDAO facilityDao;
        
        // 使用广度优先搜索（BFS）追踪连通的管线
        QQueue<QString> toProcess;
        QSet<QString> visited;
        
        toProcess.enqueue(pipelineId);
        visited.insert(pipelineId);
        
        while (!toProcess.isEmpty()) {
            QString currentPipelineId = toProcess.dequeue();
            tracedPipelines.append(currentPipelineId);
            
            // 获取当前管线的信息
            Pipeline currentPipeline = pipelineDao.findByPipelineId(currentPipelineId);
            if (!currentPipeline.isValid()) continue;
            
            // 查找该管线两端的设施（阀门、接头等）
            QPointF startPoint = currentPipeline.coordinates().first();
            QPointF endPoint = currentPipeline.coordinates().last();
            
            // 查找与起点相连的设施
            QVector<Facility> facilities = facilityDao.findNearPoint(startPoint.x(), startPoint.y(), 50.0, 10);
            facilities.append(facilityDao.findNearPoint(endPoint.x(), endPoint.y(), 50.0, 10));
            
            // 查找与这些设施相连的其他管线
            for (const Facility &facility : facilities) {
                if (facility.pipelineId().isEmpty()) continue;
                
                // 检查是否已访问
                if (!visited.contains(facility.pipelineId())) {
                    // 检查这条管线是否在爆管范围内（距离爆管点不超过搜索半径）
                    Pipeline connectedPipeline = pipelineDao.findByPipelineId(facility.pipelineId());
                    
                    bool inRange = false;
                    for (const QPointF &coord : connectedPipeline.coordinates()) {
                        double dist = std::sqrt(std::pow(coord.x() - burstPoint.x(), 2) +
                                              std::pow(coord.y() - burstPoint.y(), 2)) * 111000;
                        if (dist <= m_searchRadius) {
                            inRange = true;
                            break;
                        }
                    }
                    
                    if (inRange) {
                        visited.insert(facility.pipelineId());
                        toProcess.enqueue(facility.pipelineId());
                    }
                }
            }
        }
        
        LOG_INFO(QString("Traced %1 connected pipelines").arg(tracedPipelines.size()));
        qDebug() << "[BurstAnalyzer] Traced" << tracedPipelines.size() << "connected pipelines";
        
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Error tracing connected pipelines: %1").arg(e.what()));
        qWarning() << "[BurstAnalyzer] Error:" << e.what();
    }
    
    return tracedPipelines;
}

QPolygonF BurstAnalyzer::calculateAffectedArea(const QList<QString> &pipelines)
{
    if (pipelines.isEmpty()) {
        return QPolygonF();
    }
    
    qDebug() << "[BurstAnalyzer] Calculating affected area for" << pipelines.size() << "pipelines";
    
    try {
        PipelineDAO pipelineDao;
        QList<QPointF> allCoordinates;
        
        // 收集所有受影响管线的坐标
        for (const QString &pipelineId : pipelines) {
            Pipeline pipeline = pipelineDao.findByPipelineId(pipelineId);
            if (pipeline.isValid()) {
                allCoordinates.append(pipeline.coordinates().toList());
            }
        }
        
        if (allCoordinates.isEmpty()) {
            return QPolygonF();
        }
        
        // 基于管线坐标创建缓冲区（500米）
        // 简化实现：以所有坐标的包含矩形为基础，扩展缓冲距离
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const QPointF &coord : allCoordinates) {
            minX = qMin(minX, coord.x());
            maxX = qMax(maxX, coord.x());
            minY = qMin(minY, coord.y());
            maxY = qMax(maxY, coord.y());
        }
        
        // 计算缓冲距离（转换为度数，近似：1度 ≈ 111km）
        double bufferDegrees = (m_searchRadius / 1000.0) / 111.0;
        
        // 创建矩形缓冲区
        QPolygonF area;
        area << QPointF(minX - bufferDegrees, minY - bufferDegrees);
        area << QPointF(maxX + bufferDegrees, minY - bufferDegrees);
        area << QPointF(maxX + bufferDegrees, maxY + bufferDegrees);
        area << QPointF(minX - bufferDegrees, maxY + bufferDegrees);
        
        LOG_INFO(QString("Calculated affected area covering %1 pipelines").arg(pipelines.size()));
        qDebug() << "[BurstAnalyzer] Affected area polygon points:" << area.size();
        
        return area;
        
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Error calculating affected area: %1").arg(e.what()));
        qWarning() << "[BurstAnalyzer] Error:" << e.what();
        return QPolygonF();
    }
}

int BurstAnalyzer::estimateAffectedUsers(const QPolygonF &area)
{
    if (area.isEmpty()) {
        return 0;
    }
    
    // 计算面积（平方米）
    double areaSqM = calculatePolygonArea(area);
    
    // 转换为平方公里
    double areaSqKm = areaSqM / 1000000.0;
    
    // 根据用户密度估算
    int users = static_cast<int>(areaSqKm * m_userDensity);
    
    qDebug() << "[BurstAnalyzer] Affected area:" << areaSqKm << "km², estimated users:" << users;
    
    return users;
}

QList<QString> BurstAnalyzer::generateRepairSuggestions(const BurstAnalysisResult &result)
{
    QList<QString> suggestions;
    
    suggestions << "1. 立即通知调度中心和抢修队";
    suggestions << QString("2. 关闭以下阀门隔离爆管管段：%1个阀门").arg(result.affectedValves.size());
    
    for (const QString &valve : result.affectedValves) {
        suggestions << QString("   - %1").arg(valve);
    }
    
    suggestions << QString("3. 通知受影响用户（约%1户）做好停水准备").arg(result.estimatedAffectedUsers);
    suggestions << "4. 准备抢修物资和设备";
    suggestions << QString("5. 预计维修时间：%.1f 小时").arg(result.estimatedRepairTime);
    
    if (result.repairPriority >= 4) {
        suggestions << "⚠️ 高优先级！建议立即组织抢修";
    }
    
    suggestions << "6. 维修完成后逐步恢复供水";
    suggestions << "7. 记录事故原因并更新管网档案";
    
    return suggestions;
}

int BurstAnalyzer::calculateRepairPriority(const QString &pipelineType, double diameter, int affectedUsers)
{
    int priority = 1;
    
    // 根据管径加分
    if (diameter >= 600) priority += 2;
    else if (diameter >= 400) priority += 1;
    
    // 根据影响用户数加分
    if (affectedUsers >= 1000) priority += 2;
    else if (affectedUsers >= 500) priority += 1;
    
    // 主干管优先级更高
    if (pipelineType.contains("主干", Qt::CaseInsensitive) || 
        pipelineType.contains("Main", Qt::CaseInsensitive)) {
        priority += 1;
    }
    
    // 确保优先级在1-5范围内
    return qBound(1, priority, 5);
}

