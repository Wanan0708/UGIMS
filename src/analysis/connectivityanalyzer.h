#ifndef CONNECTIVITYANALYZER_H
#define CONNECTIVITYANALYZER_H

#include "spatialanalyzer.h"
#include <QPointF>
#include <QString>
#include <QList>
#include <QPair>

/**
 * @brief 连通性分析类型
 */
enum class ConnectivityType {
    Upstream,       // 上游追踪
    Downstream,     // 下游追踪
    ShortestPath,   // 最短路径
    AllPaths,       // 所有路径
    NetworkLoop     // 环网检查
};

/**
 * @brief 连通性分析结果
 */
struct ConnectivityResult {
    bool success;
    QString message;
    ConnectivityType type;
    
    // 起点和终点
    QString startNodeId;
    QString endNodeId;
    QPointF startPoint;
    QPointF endPoint;
    
    // 路径信息
    QList<QString> pathNodes;      // 路径节点ID列表
    QList<QString> pathPipelines;  // 路径管线ID列表
    double totalLength;            // 总长度(m)
    int nodeCount;                 // 节点数量
    
    // 分析数据
    bool isConnected;              // 是否连通
    bool hasLoop;                  // 是否存在环网
    QList<QString> disconnectedNodes; // 孤立节点
    
    ConnectivityResult() 
        : success(false)
        , type(ConnectivityType::Upstream)
        , totalLength(0)
        , nodeCount(0)
        , isConnected(false)
        , hasLoop(false)
    {}
};

/**
 * @brief 连通性分析器
 * 
 * 分析管网的连通性，包括上下游追踪、路径查找等
 */
class ConnectivityAnalyzer : public SpatialAnalyzer
{
    Q_OBJECT

public:
    explicit ConnectivityAnalyzer(QObject *parent = nullptr);
    ~ConnectivityAnalyzer() override;

    /**
     * @brief 上游追踪 - 从点追溯到源头（如水厂）
     */
    ConnectivityResult traceUpstream(const QPointF &startPoint, int maxDepth = 100);
    
    /**
     * @brief 下游追踪 - 从点追踪到末端（如污水处理厂）
     */
    ConnectivityResult traceDownstream(const QPointF &startPoint, int maxDepth = 100);
    
    /**
     * @brief 查找两点间的最短路径
     */
    ConnectivityResult findShortestPath(const QPointF &startPoint, const QPointF &endPoint);
    
    /**
     * @brief 查找两点间的所有路径
     */
    QList<ConnectivityResult> findAllPaths(const QPointF &startPoint, const QPointF &endPoint, int maxPaths = 10);
    
    /**
     * @brief 检查网络连通性
     */
    ConnectivityResult checkNetworkConnectivity();
    
    /**
     * @brief 检测环网
     */
    QList<QList<QString>> detectLoops();

signals:
    /**
     * @brief 连通性分析完成信号
     */
    void connectivityAnalysisFinished(const ConnectivityResult &result);

private:
    /**
     * @brief 使用深度优先搜索追踪管网
     */
    void dfsTrace(const QString &nodeId, int depth, int maxDepth, 
                  bool upstream, QList<QString> &visited, ConnectivityResult &result);
    
    /**
     * @brief 使用DFS查找所有路径（用于findAllPaths）
     */
    void dfsFindAllPaths(const QString &current, const QString &target,
                         QList<QString> &currentPath, QSet<QString> &visited,
                         QList<QList<QString>> &allPaths, int maxPaths);
    
    /**
     * @brief 使用Dijkstra算法查找最短路径
     */
    ConnectivityResult dijkstraShortestPath(const QString &startNodeId, const QString &endNodeId);
    
    /**
     * @brief 查找节点的相邻节点
     */
    QList<QString> getAdjacentNodes(const QString &nodeId, bool upstream);
    
    /**
     * @brief 计算两个节点之间的距离
     */
    double getDistance(const QString &fromNodeId, const QString &toNodeId);
    
    /**
     * @brief 查找连接到指定点的管线
     */
    QString findPipelineAtPoint(const QPointF &point, double toleranceMeters = 10.0);
    
    /**
     * @brief 查找连接到指定管线端点的其他管线
     */
    QList<QString> findConnectedPipelines(const QString &pipelineId, const QPointF &endpoint, double toleranceMeters = 5.0);
    
    /**
     * @brief 判断两个点是否在容差范围内（米）
     */
    bool pointsAreClose(const QPointF &p1, const QPointF &p2, double toleranceMeters = 5.0);
    
    /**
     * @brief 获取管线的起点和终点
     */
    QPair<QPointF, QPointF> getPipelineEndpoints(const QString &pipelineId);
    
    /**
     * @brief 使用BFS进行连通性追踪
     */
    ConnectivityResult bfsTrace(const QPointF &startPoint, bool upstream, int maxDepth);

private:
    int m_maxSearchDepth;  // 最大搜索深度
    double m_connectionTolerance;  // 连接容差（米）
};

#endif // CONNECTIVITYANALYZER_H

