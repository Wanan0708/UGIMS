#include "burstanalyzer.h"
#include <QDebug>
#include <QtMath>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

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
    // TODO: 实际实现需要查询数据库
    // 这里返回模拟数据
    qDebug() << "[BurstAnalyzer] Finding nearest pipeline to point:" << point;
    
    // 模拟：生成一个测试管线ID
    return QString("PIPE-%1-%2").arg(int(point.x())).arg(int(point.y()));
}

QList<QString> BurstAnalyzer::findAffectedValves(const QString &pipelineId, const QPointF &burstPoint)
{
    Q_UNUSED(pipelineId);
    Q_UNUSED(burstPoint);
    
    // TODO: 实际实现需要查询数据库
    // 查找距离爆管点在maxValveDistance范围内的阀门
    
    // 模拟数据
    return QList<QString>{
        "VALVE-001 (距离爆管点 250m, 上游)",
        "VALVE-002 (距离爆管点 300m, 下游)",
        "VALVE-003 (距离爆管点 450m, 支线)"
    };
}

QList<QString> BurstAnalyzer::traceConnectedPipelines(const QString &pipelineId, const QPointF &burstPoint)
{
    Q_UNUSED(pipelineId);
    Q_UNUSED(burstPoint);
    
    // TODO: 实际实现需要进行图遍历
    // 从爆管点开始，追踪到最近的阀门之间的所有管线
    
    // 模拟数据
    return QList<QString>{
        pipelineId,
        pipelineId + "-BRANCH-1",
        pipelineId + "-BRANCH-2"
    };
}

QPolygonF BurstAnalyzer::calculateAffectedArea(const QList<QString> &pipelines)
{
    if (pipelines.isEmpty()) {
        return QPolygonF();
    }
    
    // TODO: 实际实现需要基于管线几何形状计算
    // 这里创建一个简化的圆形影响区域
    
    QPointF center(0, 0); // 在实际实现中应该是爆管点
    return createCircleBuffer(center, m_searchRadius);
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

