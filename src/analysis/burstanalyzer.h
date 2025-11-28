#ifndef BURSTANALYZER_H
#define BURSTANALYZER_H

#include "spatialanalyzer.h"
#include <QPointF>
#include <QString>
#include <QList>

/**
 * @brief 爆管分析结果
 */
struct BurstAnalysisResult {
    bool success;
    QString message;
    
    // 爆管信息
    QPointF burstLocation;        // 爆管位置
    QString pipelineId;           // 管线ID
    QString pipelineType;         // 管线类型
    double pipelineDiameter;      // 管径(mm)
    
    // 影响范围
    QList<QString> affectedValves;     // 需要关闭的阀门列表
    QList<QString> affectedPipelines;  // 受影响的管线
    QPolygonF affectedArea;            // 受影响区域
    int estimatedAffectedUsers;        // 估计影响用户数
    
    // 分析数据
    double affectedLength;        // 受影响管线长度(m)
    double affectedAreaSize;      // 受影响面积(m²)
    int repairPriority;           // 维修优先级(1-5)
    double estimatedRepairTime;   // 预计维修时间(小时)
    
    // 建议操作
    QList<QString> suggestedActions;   // 建议操作步骤
    QList<QString> emergencyContacts;  // 应急联系人
    
    BurstAnalysisResult() 
        : success(false)
        , pipelineDiameter(0)
        , estimatedAffectedUsers(0)
        , affectedLength(0)
        , affectedAreaSize(0)
        , repairPriority(0)
        , estimatedRepairTime(0) 
    {}
};

/**
 * @brief 爆管影响分析器
 * 
 * 分析管线爆管后的影响范围、需要关闭的阀门、受影响用户等
 */
class BurstAnalyzer : public SpatialAnalyzer
{
    Q_OBJECT

public:
    explicit BurstAnalyzer(QObject *parent = nullptr);
    ~BurstAnalyzer() override;

    /**
     * @brief 执行爆管影响分析
     * @param burstPoint 爆管位置（经纬度）
     * @param pipelineId 爆管管线ID（可选，如果为空则自动查找）
     * @return 分析结果
     */
    BurstAnalysisResult analyzeBurst(const QPointF &burstPoint, const QString &pipelineId = QString());
    
    /**
     * @brief 异步执行爆管影响分析
     */
    void analyzeBurstAsync(const QPointF &burstPoint, const QString &pipelineId = QString());
    
    /**
     * @brief 设置分析参数
     */
    void setSearchRadius(double radius);        // 设置搜索半径（米）
    void setMaxValveDistance(double distance);  // 设置最大阀门距离（米）
    void setUserDensity(int usersPerKm2);      // 设置用户密度（户/平方公里）

signals:
    /**
     * @brief 爆管分析完成信号
     */
    void burstAnalysisFinished(const BurstAnalysisResult &result);

private:
    /**
     * @brief 查找离爆管点最近的管线
     */
    QString findNearestPipeline(const QPointF &point);
    
    /**
     * @brief 查找需要关闭的阀门
     */
    QList<QString> findAffectedValves(const QString &pipelineId, const QPointF &burstPoint);
    
    /**
     * @brief 追踪连通的管线
     */
    QList<QString> traceConnectedPipelines(const QString &pipelineId, const QPointF &burstPoint);
    
    /**
     * @brief 计算影响区域
     */
    QPolygonF calculateAffectedArea(const QList<QString> &pipelines);
    
    /**
     * @brief 估计受影响用户数
     */
    int estimateAffectedUsers(const QPolygonF &area);
    
    /**
     * @brief 生成维修建议
     */
    QList<QString> generateRepairSuggestions(const BurstAnalysisResult &result);
    
    /**
     * @brief 计算维修优先级（1-5，5最高）
     */
    int calculateRepairPriority(const QString &pipelineType, double diameter, int affectedUsers);

private:
    double m_searchRadius;          // 搜索半径（米）
    double m_maxValveDistance;      // 最大阀门距离（米）
    int m_userDensity;              // 用户密度（户/km²）
};

#endif // BURSTANALYZER_H

