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
#include <functional>

ConnectivityAnalyzer::ConnectivityAnalyzer(QObject *parent)
    : SpatialAnalyzer(parent)
    , m_maxSearchDepth(100)
    , m_connectionTolerance(20.0)  // 默认容差调大到20米，避免轻微偏移导致找不到
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
    qDebug() << "[ConnectivityAnalyzer] Finding all paths from:" << startPoint << "to:" << endPoint;
    
    QList<ConnectivityResult> results;
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        ConnectivityResult errorResult;
        errorResult.success = false;
        errorResult.message = "数据库未连接";
        results.append(errorResult);
        return results;
    }
    
    // 1. 找到起点和终点附近的管线
    QString startPipelineId = findPipelineAtPoint(startPoint, m_connectionTolerance * 2);
    QString endPipelineId = findPipelineAtPoint(endPoint, m_connectionTolerance * 2);
    
    if (startPipelineId.isEmpty()) {
        ConnectivityResult errorResult;
        errorResult.success = false;
        errorResult.message = "未找到起点附近的管线";
        results.append(errorResult);
        return results;
    }
    
    if (endPipelineId.isEmpty()) {
        ConnectivityResult errorResult;
        errorResult.success = false;
        errorResult.message = "未找到终点附近的管线";
        results.append(errorResult);
        return results;
    }
    
    if (startPipelineId == endPipelineId) {
        // 起点和终点在同一管线
        ConnectivityResult result;
        result.type = ConnectivityType::AllPaths;
        result.startPoint = startPoint;
        result.endPoint = endPoint;
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
        results.append(result);
        return results;
    }
    
    // 2. 使用DFS查找所有路径
    QList<QString> currentPath;
    QSet<QString> visited;
    QList<QList<QString>> allPaths;
    
    dfsFindAllPaths(startPipelineId, endPipelineId, currentPath, visited, allPaths, maxPaths);
    
    // 3. 转换为ConnectivityResult
    PipelineDAO dao;
    for (const QList<QString> &path : allPaths) {
        ConnectivityResult result;
        result.type = ConnectivityType::AllPaths;
        result.startPoint = startPoint;
        result.endPoint = endPoint;
        result.startNodeId = startPipelineId;
        result.endNodeId = endPipelineId;
        result.pathPipelines = path;
        result.nodeCount = path.size();
        result.isConnected = !path.isEmpty();
        result.success = true;
        
        // 计算总长度
        double totalLength = 0.0;
        for (const QString &pid : path) {
            Pipeline p = dao.findByPipelineId(pid);
            if (p.isValid()) {
                totalLength += p.lengthM();
            }
        }
        result.totalLength = totalLength;
        
        results.append(result);
    }
    
    if (results.isEmpty()) {
        ConnectivityResult errorResult;
        errorResult.success = false;
        errorResult.message = "未找到从起点到终点的路径";
        results.append(errorResult);
    }
    
    qDebug() << "[ConnectivityAnalyzer] Found" << results.size() << "paths";
    return results;
}

ConnectivityResult ConnectivityAnalyzer::checkNetworkConnectivity()
{
    qDebug() << "[ConnectivityAnalyzer] Checking network connectivity";
    
    ConnectivityResult result;
    result.type = ConnectivityType::NetworkLoop;
    
    try {
        PipelineDAO dao;
        
        // 获取所有管线（限制数量以避免性能问题）
        QVector<Pipeline> allPipelines = dao.findByBounds(QRectF(-180, -90, 360, 180), 10000);
        
        if (allPipelines.isEmpty()) {
            result.success = false;
            result.message = "未找到管网数据";
            return result;
        }
        
        // 构建图：管线ID -> 连接的管线ID列表
        QMap<QString, QSet<QString>> graph;
        QSet<QString> allNodes;
        
        for (const Pipeline &pipeline : allPipelines) {
            QString pid = pipeline.pipelineId();
            allNodes.insert(pid);
            graph[pid] = QSet<QString>();
            
            // 查找连接的管线
            QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(pid);
            if (endpoints.first.isNull() || endpoints.second.isNull()) {
                continue;
            }
            
            QList<QString> connected1 = findConnectedPipelines(pid, endpoints.first, m_connectionTolerance);
            QList<QString> connected2 = findConnectedPipelines(pid, endpoints.second, m_connectionTolerance);
            
            for (const QString &connected : connected1) {
                graph[pid].insert(connected);
            }
            for (const QString &connected : connected2) {
                graph[pid].insert(connected);
            }
        }
        
        // 使用DFS检查连通性
        QSet<QString> visited;
        QList<QString> disconnectedComponents;
        
        for (const QString &node : allNodes) {
            if (!visited.contains(node)) {
                // 新的连通分量
                QSet<QString> component;
                QQueue<QString> queue;
                queue.enqueue(node);
                visited.insert(node);
                component.insert(node);
                
                while (!queue.isEmpty()) {
                    QString current = queue.dequeue();
                    for (const QString &neighbor : graph.value(current)) {
                        if (!visited.contains(neighbor)) {
                            visited.insert(neighbor);
                            component.insert(neighbor);
                            queue.enqueue(neighbor);
                        }
                    }
                }
                
                if (component.size() == 1) {
                    disconnectedComponents.append(*component.begin());
                }
            }
        }
        
        // 检测环网：使用DFS检测回边
        bool hasLoop = false;
        QSet<QString> recStack;
        QSet<QString> visitedForLoop;
        
        std::function<bool(const QString&)> hasCycle = [&](const QString &node) -> bool {
            visitedForLoop.insert(node);
            recStack.insert(node);
            
            for (const QString &neighbor : graph.value(node)) {
                if (!visitedForLoop.contains(neighbor)) {
                    if (hasCycle(neighbor)) {
                        return true;
                    }
                } else if (recStack.contains(neighbor)) {
                    return true; // 发现回边，存在环
                }
            }
            
            recStack.remove(node);
            return false;
        };
        
        for (const QString &node : allNodes) {
            if (!visitedForLoop.contains(node)) {
                if (hasCycle(node)) {
                    hasLoop = true;
                    break;
                }
            }
        }
        
        result.isConnected = (disconnectedComponents.size() <= 1);
        result.hasLoop = hasLoop;
        result.nodeCount = allNodes.size();
        result.disconnectedNodes = disconnectedComponents;
        result.success = true;
        result.message = QString("网络连通性检查完成：%1个节点，%2个孤立节点，%3")
                            .arg(allNodes.size())
                            .arg(disconnectedComponents.size())
                            .arg(hasLoop ? "存在环网" : "无环网");
        
    } catch (const std::exception &e) {
        qWarning() << "[ConnectivityAnalyzer] Network connectivity check error:" << e.what();
        result.success = false;
        result.message = QString("检查失败: %1").arg(e.what());
    }
    
    return result;
}

QList<QList<QString>> ConnectivityAnalyzer::detectLoops()
{
    qDebug() << "[ConnectivityAnalyzer] Detecting loops";
    
    QList<QList<QString>> loops;
    
    try {
        PipelineDAO dao;
        
        // 获取所有管线
        QVector<Pipeline> allPipelines = dao.findByBounds(QRectF(-180, -90, 360, 180), 10000);
        
        if (allPipelines.isEmpty()) {
            return loops;
        }
        
        // 构建图
        QMap<QString, QSet<QString>> graph;
        QSet<QString> allNodes;
        
        for (const Pipeline &pipeline : allPipelines) {
            QString pid = pipeline.pipelineId();
            allNodes.insert(pid);
            graph[pid] = QSet<QString>();
            
            QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(pid);
            if (endpoints.first.isNull() || endpoints.second.isNull()) {
                continue;
            }
            
            QList<QString> connected1 = findConnectedPipelines(pid, endpoints.first, m_connectionTolerance);
            QList<QString> connected2 = findConnectedPipelines(pid, endpoints.second, m_connectionTolerance);
            
            for (const QString &connected : connected1) {
                graph[pid].insert(connected);
            }
            for (const QString &connected : connected2) {
                graph[pid].insert(connected);
            }
        }
        
        // 使用DFS查找所有环
        QSet<QString> visited;
        QList<QString> currentPath;
        QSet<QPair<QString, QString>> processedEdges; // 避免重复处理边
        
        std::function<void(const QString&, const QString&)> findCycles = 
            [&](const QString &node, const QString &parent) {
                if (visited.contains(node)) {
                    // 找到环：从node到当前路径的起点
                    int startIdx = currentPath.indexOf(node);
                    if (startIdx >= 0 && currentPath.size() - startIdx >= 3) {
                        QList<QString> cycle;
                        for (int i = startIdx; i < currentPath.size(); ++i) {
                            cycle.append(currentPath[i]);
                        }
                        cycle.append(node); // 闭合环
                        
                        // 检查是否已存在相同的环（不同方向）
                        bool exists = false;
                        for (const QList<QString> &existingLoop : loops) {
                            if (existingLoop.size() == cycle.size()) {
                                // 检查是否是同一个环（可能是不同方向）
                                bool same = true;
                                for (int i = 0; i < cycle.size(); ++i) {
                                    if (!existingLoop.contains(cycle[i])) {
                                        same = false;
                                        break;
                                    }
                                }
                                if (same) {
                                    exists = true;
                                    break;
                                }
                            }
                        }
                        
                        if (!exists) {
                            loops.append(cycle);
                        }
                    }
                    return;
                }
                
                visited.insert(node);
                currentPath.append(node);
                
                for (const QString &neighbor : graph.value(node)) {
                    if (neighbor != parent) {
                        QPair<QString, QString> edge(qMin(node, neighbor), qMax(node, neighbor));
                        if (!processedEdges.contains(edge)) {
                            processedEdges.insert(edge);
                            findCycles(neighbor, node);
                        }
                    }
                }
                
                currentPath.removeLast();
                visited.remove(node);
            };
        
        // 从每个未访问的节点开始查找环
        for (const QString &node : allNodes) {
            if (!visited.contains(node)) {
                findCycles(node, QString());
            }
        }
        
        qDebug() << "[ConnectivityAnalyzer] Found" << loops.size() << "loops";
        
    } catch (const std::exception &e) {
        qWarning() << "[ConnectivityAnalyzer] Loop detection error:" << e.what();
    }
    
    return loops;
}

// ========================================
// 私有辅助方法
// ========================================

void ConnectivityAnalyzer::dfsTrace(const QString &nodeId, int depth, int maxDepth, 
                                     bool upstream, QList<QString> &visited, ConnectivityResult &result)
{
    // 检查深度限制
    if (depth > maxDepth) {
        return;
    }
    
    // 检查是否已访问（避免环路）
    if (visited.contains(nodeId)) {
        return;
    }
    
    // 标记为已访问
    visited.append(nodeId);
    result.pathPipelines.append(nodeId);
    
    // 获取相邻节点
    QList<QString> adjacent = getAdjacentNodes(nodeId, upstream);
    
    // 递归访问相邻节点
    for (const QString &adjNode : adjacent) {
        if (!visited.contains(adjNode)) {
            dfsTrace(adjNode, depth + 1, maxDepth, upstream, visited, result);
        }
    }
}

void ConnectivityAnalyzer::dfsFindAllPaths(const QString &current, const QString &target,
                                            QList<QString> &currentPath, QSet<QString> &visited,
                                            QList<QList<QString>> &allPaths, int maxPaths)
{
    // 如果已找到足够多的路径，停止搜索
    if (allPaths.size() >= maxPaths) {
        return;
    }
    
    // 将当前节点加入路径
    currentPath.append(current);
    visited.insert(current);
    
    // 如果到达目标，保存路径
    if (current == target) {
        allPaths.append(currentPath);
        currentPath.removeLast();
        visited.remove(current);
        return;
    }
    
    // 获取当前管线的端点
    QPair<QPointF, QPointF> endpoints = getPipelineEndpoints(current);
    if (endpoints.first.isNull() || endpoints.second.isNull()) {
        currentPath.removeLast();
        visited.remove(current);
        return;
    }
    
    // 查找连接的管线
    QList<QString> connected1 = findConnectedPipelines(current, endpoints.first, m_connectionTolerance);
    QList<QString> connected2 = findConnectedPipelines(current, endpoints.second, m_connectionTolerance);
    
    QSet<QString> neighbors;
    for (const QString &p : connected1) neighbors.insert(p);
    for (const QString &p : connected2) neighbors.insert(p);
    neighbors.remove(current); // 移除自身
    
    // 递归访问未访问的邻居
    for (const QString &neighbor : neighbors) {
        if (!visited.contains(neighbor)) {
            dfsFindAllPaths(neighbor, target, currentPath, visited, allPaths, maxPaths);
        }
    }
    
    // 回溯：移除当前节点
    currentPath.removeLast();
    visited.remove(current);
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
    QList<QString> adjacent;
    
    // 将nodeId视为管线ID，查找连接的管线
    PipelineDAO dao;
    Pipeline pipeline = dao.findByPipelineId(nodeId);
    
    if (!pipeline.isValid() || pipeline.coordinates().isEmpty()) {
        return adjacent;
    }
    
    QVector<QPointF> coords = pipeline.coordinates();
    QPointF searchEndpoint = upstream ? coords.first() : coords.last();
    
    // 查找连接到端点的管线
    QList<QString> connected = findConnectedPipelines(nodeId, searchEndpoint, m_connectionTolerance);
    adjacent = connected;
    
    return adjacent;
}

double ConnectivityAnalyzer::getDistance(const QString &fromNodeId, const QString &toNodeId)
{
    PipelineDAO dao;
    Pipeline fromPipeline = dao.findByPipelineId(fromNodeId);
    Pipeline toPipeline = dao.findByPipelineId(toNodeId);
    
    if (!fromPipeline.isValid() || !toPipeline.isValid()) {
        return std::numeric_limits<double>::max();
    }
    
    // 获取两个管线的端点
    QPair<QPointF, QPointF> fromEndpoints = getPipelineEndpoints(fromNodeId);
    QPair<QPointF, QPointF> toEndpoints = getPipelineEndpoints(toNodeId);
    
    if (fromEndpoints.first.isNull() || toEndpoints.first.isNull()) {
        return std::numeric_limits<double>::max();
    }
    
    // 计算两个管线之间的最短距离（端点之间的距离）
    double minDist = std::numeric_limits<double>::max();
    
    QList<QPointF> fromPoints = {fromEndpoints.first, fromEndpoints.second};
    QList<QPointF> toPoints = {toEndpoints.first, toEndpoints.second};
    
    for (const QPointF &fromPoint : fromPoints) {
        for (const QPointF &toPoint : toPoints) {
            double dist = calculateDistance(fromPoint.y(), fromPoint.x(), toPoint.y(), toPoint.x());
            if (dist < minDist) {
                minDist = dist;
            }
        }
    }
    
    // 如果距离在容差范围内，返回管线长度之和的一半（近似）
    if (minDist <= m_connectionTolerance) {
        return (fromPipeline.lengthM() + toPipeline.lengthM()) / 2.0;
    }
    
    return minDist;
}

QString ConnectivityAnalyzer::findPipelineAtPoint(const QPointF &point, double toleranceMeters)
{
    PipelineDAO dao;

    auto findWithTolerance = [&](double tol) -> QString {
        QVector<Pipeline> nearby = dao.findNearPoint(point.x(), point.y(), tol, 10);
        if (nearby.isEmpty()) return QString();
        return nearby.first().pipelineId();
    };

    // 1st attempt: 使用给定容差
    QString id = findWithTolerance(toleranceMeters);
    if (!id.isEmpty()) return id;

    // 2nd attempt: 放宽容差到 50 米，兼容底图/数据轻微偏移
    id = findWithTolerance(qMax(toleranceMeters, 50.0));
    return id;
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

