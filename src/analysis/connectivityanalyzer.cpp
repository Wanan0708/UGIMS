#include "connectivityanalyzer.h"
#include "dao/pipelinedao.h"
#include "core/database/databasemanager.h"
#include "core/common/logger.h"
#include <QDebug>
#include <QQueue>
#include <QSet>
#include <QMap>
#include <QtMath>
#include <QPair>
#include <limits>

ConnectivityAnalyzer::ConnectivityAnalyzer(QObject *parent)
    : SpatialAnalyzer(parent)
    , m_maxSearchDepth(100)
    , m_connectionTolerance(5.0)  // 默认5米容差
{
}

ConnectivityAnalyzer::~ConnectivityAnalyzer()
{
}

ConnectivityResult ConnectivityAnalyzer::traceUpstream(const QPointF &startPoint, int maxDepth)
{
    qDebug() << "[ConnectivityAnalyzer] Tracing upstream from:" << startPoint;
    
    ConnectivityResult result;
    result.type = ConnectivityType::Upstream;
    result.startPoint = startPoint;
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        result.success = false;
        result.message = "数据库未连接";
        return result;
    }
    
    // 使用BFS进行上游追踪
    result = bfsTrace(startPoint, true, maxDepth);
    
    return result;
}

ConnectivityResult ConnectivityAnalyzer::traceDownstream(const QPointF &startPoint, int maxDepth)
{
    qDebug() << "[ConnectivityAnalyzer] Tracing downstream from:" << startPoint;
    
    ConnectivityResult result;
    result.type = ConnectivityType::Downstream;
    result.startPoint = startPoint;
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        result.success = false;
        result.message = "数据库未连接";
        return result;
    }
    
    // 使用BFS进行下游追踪
    result = bfsTrace(startPoint, false, maxDepth);
    
    return result;
}

ConnectivityResult ConnectivityAnalyzer::findShortestPath(const QPointF &startPoint, const QPointF &endPoint)
{
    qDebug() << "[ConnectivityAnalyzer] Finding shortest path from:" << startPoint << "to:" << endPoint;
    
    ConnectivityResult result;
    result.type = ConnectivityType::ShortestPath;
    result.startPoint = startPoint;
    result.endPoint = endPoint;
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        result.success = false;
        result.message = "数据库未连接";
        return result;
    }
    
    // 1. 找到起点和终点附近的管线
    QString startPipelineId = findPipelineAtPoint(startPoint, m_connectionTolerance * 2);
    QString endPipelineId = findPipelineAtPoint(endPoint, m_connectionTolerance * 2);
    
    if (startPipelineId.isEmpty()) {
        result.success = false;
        result.message = "未找到起点附近的管线";
        return result;
    }
    
    if (endPipelineId.isEmpty()) {
        result.success = false;
        result.message = "未找到终点附近的管线";
        return result;
    }
    
    if (startPipelineId == endPipelineId) {
        // 起点和终点在同一管线
        result.startNodeId = startPipelineId;
        result.endNodeId = endPipelineId;
        result.pathPipelines.append(startPipelineId);
        PipelineDAO dao;
        Pipeline pipeline = dao.findByPipelineId(startPipelineId);
        if (pipeline.isValid()) {
            result.totalLength = pipeline.lengthM();
        }
        result.nodeCount = 1;
        result.isConnected = true;
        result.success = true;
        result.message = "起点和终点在同一管线";
        return result;
    }
    
    // 2. 使用Dijkstra算法查找最短路径
    result = dijkstraShortestPath(startPipelineId, endPipelineId);
    
    if (!result.success) {
        return result;
    }
    
    result.startPoint = startPoint;
    result.endPoint = endPoint;
    result.message = QString("最短路径查找完成，路径长度: %1 m").arg(result.totalLength, 0, 'f', 1);
    
    return result;
}

QList<ConnectivityResult> ConnectivityAnalyzer::findAllPaths(const QPointF &startPoint, const QPointF &endPoint, int maxPaths)
{
    Q_UNUSED(maxPaths);
    
    qDebug() << "[ConnectivityAnalyzer] Finding all paths from:" << startPoint << "to:" << endPoint;
    
    QList<ConnectivityResult> results;
    
    // TODO: 实际实现需要使用图遍历算法
    // 返回模拟数据
    
    ConnectivityResult path1;
    path1.type = ConnectivityType::AllPaths;
    path1.startPoint = startPoint;
    path1.endPoint = endPoint;
    path1.pathNodes = {"NODE-START", "NODE-A", "NODE-B", "NODE-END"};
    path1.totalLength = 1500.0;
    path1.success = true;
    results.append(path1);
    
    ConnectivityResult path2;
    path2.type = ConnectivityType::AllPaths;
    path2.startPoint = startPoint;
    path2.endPoint = endPoint;
    path2.pathNodes = {"NODE-START", "NODE-C", "NODE-D", "NODE-END"};
    path2.totalLength = 1800.0;
    path2.success = true;
    results.append(path2);
    
    return results;
}

ConnectivityResult ConnectivityAnalyzer::checkNetworkConnectivity()
{
    qDebug() << "[ConnectivityAnalyzer] Checking network connectivity";
    
    ConnectivityResult result;
    result.type = ConnectivityType::NetworkLoop;
    
    // TODO: 实际实现需要遍历整个网络
    
    result.isConnected = true;
    result.hasLoop = true;
    result.nodeCount = 150;
    result.success = true;
    result.message = "网络连通性检查完成";
    
    return result;
}

QList<QList<QString>> ConnectivityAnalyzer::detectLoops()
{
    qDebug() << "[ConnectivityAnalyzer] Detecting loops";
    
    QList<QList<QString>> loops;
    
    // TODO: 实际实现需要使用环检测算法
    // 返回模拟数据
    
    loops.append({"NODE-A", "NODE-B", "NODE-C", "NODE-A"});
    loops.append({"NODE-D", "NODE-E", "NODE-F", "NODE-D"});
    
    return loops;
}

// ========================================
// 私有辅助方法
// ========================================

void ConnectivityAnalyzer::dfsTrace(const QString &nodeId, int depth, int maxDepth, 
                                     bool upstream, QList<QString> &visited, ConnectivityResult &result)
{
    Q_UNUSED(nodeId);
    Q_UNUSED(depth);
    Q_UNUSED(maxDepth);
    Q_UNUSED(upstream);
    Q_UNUSED(visited);
    Q_UNUSED(result);
    
    // TODO: 实现深度优先搜索
}

ConnectivityResult ConnectivityAnalyzer::dijkstraShortestPath(const QString &startPipelineId, const QString &endPipelineId)
{
    ConnectivityResult result;
    result.type = ConnectivityType::ShortestPath;
    result.startNodeId = startPipelineId;
    result.endNodeId = endPipelineId;
    
    // Dijkstra算法数据结构
    QMap<QString, double> distances;  // 到每个节点的最短距离
    QMap<QString, QString> previous;   // 前驱节点，用于重建路径
    QSet<QString> unvisited;           // 未访问的节点集合
    QSet<QString> visited;             // 已访问的节点集合
    
    PipelineDAO dao;
    
    // 初始化：将所有管线加入未访问集合，距离设为无穷大
    // 首先需要获取所有相关的管线（通过BFS从起点和终点扩展）
    QSet<QString> allPipelines;
    QQueue<QString> expandQueue;
    expandQueue.enqueue(startPipelineId);
    expandQueue.enqueue(endPipelineId);
    allPipelines.insert(startPipelineId);
    allPipelines.insert(endPipelineId);
    
    // 扩展搜索范围，找到所有可能相关的管线
    int maxExpandDepth = 50; // 限制扩展深度，避免搜索整个网络
    QMap<QString, int> expandDepth;
    expandDepth[startPipelineId] = 0;
    expandDepth[endPipelineId] = 0;
    
    while (!expandQueue.isEmpty() && expandDepth.size() < 1000) { // 限制最大节点数
        QString current = expandQueue.dequeue();
        int depth = expandDepth.value(current, 0);
        
        if (depth >= maxExpandDepth) continue;
        
        QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(current);
        if (endpoints.first.isNull() || endpoints.second.isNull()) continue;
        
        // 查找连接到两个端点的管线
        QList<QString> connected1 = findConnectedPipelines(current, endpoints.first, m_connectionTolerance);
        QList<QString> connected2 = findConnectedPipelines(current, endpoints.second, m_connectionTolerance);
        
        QSet<QString> allConnected;
        for (const QString &p : connected1) allConnected.insert(p);
        for (const QString &p : connected2) allConnected.insert(p);
        
        for (const QString &next : allConnected) {
            if (!allPipelines.contains(next)) {
                allPipelines.insert(next);
                expandQueue.enqueue(next);
                expandDepth[next] = depth + 1;
            }
        }
    }
    
    // 初始化距离
    for (const QString &pid : allPipelines) {
        distances[pid] = std::numeric_limits<double>::max();
        unvisited.insert(pid);
    }
    distances[startPipelineId] = 0.0;
    
    // Dijkstra主循环
    while (!unvisited.isEmpty()) {
        // 找到未访问节点中距离最小的
        QString current = *unvisited.begin();
        double minDist = distances.value(current, std::numeric_limits<double>::max());
        
        for (const QString &pid : unvisited) {
            if (distances.value(pid, std::numeric_limits<double>::max()) < minDist) {
                minDist = distances.value(pid);
                current = pid;
            }
        }
        
        // 如果当前节点距离为无穷大，说明无法到达
        if (minDist == std::numeric_limits<double>::max()) {
            break;
        }
        
        // 如果到达终点，可以提前结束
        if (current == endPipelineId) {
            break;
        }
        
        unvisited.remove(current);
        visited.insert(current);
        
        // 获取当前管线的长度
        Pipeline currentPipeline = dao.findByPipelineId(current);
        if (!currentPipeline.isValid()) continue;
        double currentLength = currentPipeline.lengthM();
        
        // 获取当前管线的端点
        QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(current);
        if (endpoints.first.isNull() || endpoints.second.isNull()) continue;
        
        // 查找连接的管线
        QList<QString> connected1 = findConnectedPipelines(current, endpoints.first, m_connectionTolerance);
        QList<QString> connected2 = findConnectedPipelines(current, endpoints.second, m_connectionTolerance);
        
        QSet<QString> neighbors;
        for (const QString &p : connected1) neighbors.insert(p);
        for (const QString &p : connected2) neighbors.insert(p);
        
        // 更新邻居节点的距离
        for (const QString &neighbor : neighbors) {
            if (!unvisited.contains(neighbor)) continue;
            
            double alt = distances.value(current) + currentLength;
            if (alt < distances.value(neighbor, std::numeric_limits<double>::max())) {
                distances[neighbor] = alt;
                previous[neighbor] = current;
            }
        }
    }
    
    // 重建路径
    if (startPipelineId == endPipelineId) {
        // 起点和终点相同
        result.pathPipelines.append(startPipelineId);
        result.totalLength = 0.0;
        Pipeline pipeline = dao.findByPipelineId(startPipelineId);
        if (pipeline.isValid()) {
            result.totalLength = pipeline.lengthM();
        }
    } else if (!previous.contains(endPipelineId)) {
        // 无法找到路径
        result.success = false;
        result.message = "无法找到从起点到终点的路径";
        return result;
    } else {
        // 从终点回溯到起点
        QString current = endPipelineId;
        result.pathPipelines.prepend(current);
        
        while (current != startPipelineId && previous.contains(current)) {
            current = previous.value(current);
            result.pathPipelines.prepend(current);
        }
        
        // 确保起点在路径中
        if (result.pathPipelines.isEmpty() || result.pathPipelines.first() != startPipelineId) {
            result.pathPipelines.prepend(startPipelineId);
        }
    }
    
    // 计算总长度
    result.totalLength = distances.value(endPipelineId, 0.0);
    result.nodeCount = result.pathPipelines.size();
    result.isConnected = result.pathPipelines.size() > 0;
    result.success = true;
    
    qDebug() << "[ConnectivityAnalyzer] Shortest path found:" << result.pathPipelines.size() 
             << "pipelines, length:" << result.totalLength;
    
    return result;
}

QList<QString> ConnectivityAnalyzer::getAdjacentNodes(const QString &nodeId, bool upstream)
{
    Q_UNUSED(nodeId);
    Q_UNUSED(upstream);
    
    // TODO: 实际实现需要查询数据库
    return QList<QString>();
}

double ConnectivityAnalyzer::getDistance(const QString &fromNodeId, const QString &toNodeId)
{
    Q_UNUSED(fromNodeId);
    Q_UNUSED(toNodeId);
    
    // TODO: 实际实现需要查询数据库
    return 0.0;
}

QString ConnectivityAnalyzer::findPipelineAtPoint(const QPointF &point, double toleranceMeters)
{
    PipelineDAO dao;
    QVector<Pipeline> nearbyPipelines = dao.findNearPoint(point.x(), point.y(), toleranceMeters, 10);
    
    if (nearbyPipelines.isEmpty()) {
        return QString();
    }
    
    // 返回最近的管线
    return nearbyPipelines.first().pipelineId();
}

QPair<QPointF, QPointF> ConnectivityAnalyzer::getPipelineEndpoints(const QString &pipelineId)
{
    PipelineDAO dao;
    Pipeline pipeline = dao.findByPipelineId(pipelineId);
    
    if (!pipeline.isValid() || pipeline.coordinates().isEmpty()) {
        return QPair<QPointF, QPointF>();
    }
    
    QVector<QPointF> coords = pipeline.coordinates();
    return QPair<QPointF, QPointF>(coords.first(), coords.last());
}

bool ConnectivityAnalyzer::pointsAreClose(const QPointF &p1, const QPointF &p2, double toleranceMeters)
{
    double distance = calculateDistance(p1.y(), p1.x(), p2.y(), p2.x());
    return distance <= toleranceMeters;
}

QList<QString> ConnectivityAnalyzer::findConnectedPipelines(const QString &pipelineId, const QPointF &endpoint, double toleranceMeters)
{
    QList<QString> connected;
    
    // 获取当前管线的端点
    QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(pipelineId);
    if (endpoints.first.isNull() || endpoints.second.isNull()) {
        return connected;
    }
    
    // 确定要查找的连接端点（上游追踪找起点，下游追踪找终点）
    QPointF searchPoint = endpoint;
    
    // 查找附近的所有管线
    PipelineDAO dao;
    QVector<Pipeline> nearbyPipelines = dao.findNearPoint(searchPoint.x(), searchPoint.y(), toleranceMeters * 2, 100);
    
    for (const Pipeline &p : nearbyPipelines) {
        if (p.pipelineId() == pipelineId) {
            continue; // 跳过自己
        }
        
        QPair<QPointF, QPointF> otherEndpoints = getPipelineEndpoints(p.pipelineId());
        if (otherEndpoints.first.isNull() || otherEndpoints.second.isNull()) {
            continue;
        }
        
        // 检查是否与当前端点连接
        if (pointsAreClose(searchPoint, otherEndpoints.first, toleranceMeters) ||
            pointsAreClose(searchPoint, otherEndpoints.second, toleranceMeters)) {
            connected.append(p.pipelineId());
        }
    }
    
    return connected;
}

ConnectivityResult ConnectivityAnalyzer::bfsTrace(const QPointF &startPoint, bool upstream, int maxDepth)
{
    ConnectivityResult result;
    result.type = upstream ? ConnectivityType::Upstream : ConnectivityType::Downstream;
    result.startPoint = startPoint;
    
    // 1. 找到起点附近的管线
    QString startPipelineId = findPipelineAtPoint(startPoint, m_connectionTolerance * 2);
    if (startPipelineId.isEmpty()) {
        result.success = false;
        result.message = "未找到起点附近的管线";
        return result;
    }
    
    result.startNodeId = startPipelineId;
    
    // 2. 获取起点管线的端点
    QPair<QPointF, QPointF> startEndpoints = getPipelineEndpoints(startPipelineId);
    if (startEndpoints.first.isNull() || startEndpoints.second.isNull()) {
        result.success = false;
        result.message = "无法获取起点管线端点";
        return result;
    }
    
    // 确定追踪方向：上游追踪从起点向起点管线的起点方向，下游追踪向终点方向
    QPointF currentEndpoint;
    if (upstream) {
        // 上游：选择距离起点更近的端点作为追踪起点
        double distToFirst = calculateDistance(startPoint.y(), startPoint.x(), 
                                               startEndpoints.first.y(), startEndpoints.first.x());
        double distToSecond = calculateDistance(startPoint.y(), startPoint.x(), 
                                                startEndpoints.second.y(), startEndpoints.second.x());
        currentEndpoint = (distToFirst < distToSecond) ? startEndpoints.first : startEndpoints.second;
    } else {
        // 下游：选择距离起点更远的端点作为追踪起点
        double distToFirst = calculateDistance(startPoint.y(), startPoint.x(), 
                                               startEndpoints.first.y(), startEndpoints.first.x());
        double distToSecond = calculateDistance(startPoint.y(), startPoint.x(), 
                                                startEndpoints.second.y(), startEndpoints.second.x());
        currentEndpoint = (distToFirst > distToSecond) ? startEndpoints.first : startEndpoints.second;
    }
    
    // 3. BFS遍历
    QQueue<QPair<QString, int>> queue;  // <pipelineId, depth>
    QSet<QString> visited;
    QMap<QString, QString> parent;  // 用于记录路径
    
    queue.enqueue(QPair<QString, int>(startPipelineId, 0));
    visited.insert(startPipelineId);
    result.pathPipelines.append(startPipelineId);
    
    PipelineDAO dao;
    double totalLength = 0.0;
    
    // 先累加起点管线长度
    Pipeline startPipeline = dao.findByPipelineId(startPipelineId);
    if (startPipeline.isValid()) {
        totalLength += startPipeline.lengthM();
    }
    
    while (!queue.isEmpty()) {
        QPair<QString, int> current = queue.dequeue();
        QString currentPipelineId = current.first;
        int depth = current.second;
        
        if (depth >= maxDepth) {
            continue;
        }
        
        // 获取当前管线的端点
        QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(currentPipelineId);
        if (endpoints.first.isNull() || endpoints.second.isNull()) {
            continue;
        }
        
        // 确定要追踪的端点
        QPointF searchEndpoint;
        if (currentPipelineId == startPipelineId) {
            // 起点管线：使用之前确定的端点
            searchEndpoint = currentEndpoint;
        } else {
            // 其他管线：找到与父管线连接的端点，然后追踪另一个端点
            QString parentId = parent.value(currentPipelineId);
            if (!parentId.isEmpty()) {
                QPair<QPointF, QPointF> parentEndpoints = getPipelineEndpoints(parentId);
                // 找到与父端点接近的端点（连接点）
                if (pointsAreClose(endpoints.first, parentEndpoints.first, m_connectionTolerance) ||
                    pointsAreClose(endpoints.first, parentEndpoints.second, m_connectionTolerance)) {
                    searchEndpoint = endpoints.second; // 追踪另一个端点（远离父管线的端点）
                } else {
                    searchEndpoint = endpoints.first;  // 追踪另一个端点
                }
            } else {
                // 没有父节点（不应该发生），使用默认端点
                searchEndpoint = (upstream) ? endpoints.first : endpoints.second;
            }
        }
        
        // 查找连接到searchEndpoint的管线
        QList<QString> connected = findConnectedPipelines(currentPipelineId, searchEndpoint, m_connectionTolerance);
        
        for (const QString &nextPipelineId : connected) {
            if (!visited.contains(nextPipelineId)) {
                visited.insert(nextPipelineId);
                queue.enqueue(QPair<QString, int>(nextPipelineId, depth + 1));
                parent[nextPipelineId] = currentPipelineId;
                result.pathPipelines.append(nextPipelineId);
                
                // 累加长度（只累加新发现的管线）
                Pipeline nextPipeline = dao.findByPipelineId(nextPipelineId);
                if (nextPipeline.isValid()) {
                    totalLength += nextPipeline.lengthM();
                }
            }
        }
    }
    
    result.totalLength = totalLength;
    result.nodeCount = result.pathPipelines.size();
    result.isConnected = result.pathPipelines.size() > 1;
    result.success = true;
    result.message = QString("%1追踪完成，找到 %2 条连通管线").arg(upstream ? "上游" : "下游").arg(result.pathPipelines.size());
    
    qDebug() << "[ConnectivityAnalyzer] Trace result:" << result.pathPipelines.size() << "pipelines, length:" << totalLength;
    
    return result;
}

