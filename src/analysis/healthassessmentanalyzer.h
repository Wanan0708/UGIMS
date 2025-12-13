#ifndef HEALTHASSESSMENTANALYZER_H
#define HEALTHASSESSMENTANALYZER_H

#include <QObject>
#include <QDate>
#include <QString>
#include <QMap>
#include <QColor>

class Pipeline;
class Facility;

/**
 * @brief 健康度评估结果
 */
struct HealthAssessmentResult {
    int score;                      // 健康度分数 (0-100)
    QString level;                  // 健康等级 (优秀/良好/一般/较差/危险)
    QString description;            // 评估描述
    QMap<QString, double> factors;  // 各因素得分详情
    
    HealthAssessmentResult() : score(100), level("优秀") {}
};

/**
 * @brief 管网健康度评估器
 * 
 * 基于多个因素评估管线和设施的健康度：
 * - 建设年限（老化程度）
 * - 材质类型（耐久性）
 * - 历史故障记录
 * - 巡检记录
 * - 环境因素
 */
class HealthAssessmentAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit HealthAssessmentAnalyzer(QObject *parent = nullptr);
    
    /**
     * @brief 评估管线健康度
     * @param pipeline 管线对象
     * @return 健康度评估结果
     */
    HealthAssessmentResult assessPipeline(const Pipeline &pipeline);
    
    /**
     * @brief 评估设施健康度
     * @param facility 设施对象
     * @return 健康度评估结果
     */
    HealthAssessmentResult assessFacility(const Facility &facility);
    
    /**
     * @brief 批量评估所有管线
     * @return 评估结果统计
     */
    QMap<QString, int> batchAssessPipelines();
    
    /**
     * @brief 批量评估所有设施
     * @return 评估结果统计
     */
    QMap<QString, int> batchAssessFacilities();
    
    /**
     * @brief 获取健康度等级
     * @param score 健康度分数
     * @return 等级名称
     */
    static QString getHealthLevel(int score);
    
    /**
     * @brief 获取健康度等级颜色
     * @param score 健康度分数
     * @return 颜色值
     */
    static QColor getHealthColor(int score);

signals:
    /**
     * @brief 评估进度信号
     * @param current 当前进度
     * @param total 总数
     */
    void assessmentProgress(int current, int total);
    
    /**
     * @brief 评估完成信号
     * @param pipelineCount 管线数量
     * @param facilityCount 设施数量
     */
    void assessmentComplete(int pipelineCount, int facilityCount);

private:
    /**
     * @brief 计算建设年限得分
     * @param buildDate 建设日期
     * @param designLife 设计寿命（年）
     * @return 得分 (0-100)
     */
    double calculateAgeScore(const QDate &buildDate, int designLife = 50) const;
    
    /**
     * @brief 计算材质得分
     * @param material 材质类型
     * @return 得分 (0-100)
     */
    double calculateMaterialScore(const QString &material) const;
    
    /**
     * @brief 计算巡检记录得分
     * @param lastInspection 上次巡检日期
     * @param inspectionCycle 巡检周期（天）
     * @return 得分 (0-100)
     */
    double calculateInspectionScore(const QDate &lastInspection, int inspectionCycle) const;
    
    /**
     * @brief 计算状态得分
     * @param status 状态字符串
     * @return 得分 (0-100)
     */
    double calculateStatusScore(const QString &status) const;
    
    /**
     * @brief 获取材质的设计寿命
     * @param material 材质类型
     * @return 设计寿命（年）
     */
    int getMaterialDesignLife(const QString &material) const;
    
    /**
     * @brief 计算综合得分
     * @param factors 各因素得分
     * @return 综合得分 (0-100)
     */
    int calculateFinalScore(const QMap<QString, double> &factors) const;
};

#endif // HEALTHASSESSMENTANALYZER_H

