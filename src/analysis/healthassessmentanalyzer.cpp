#include "healthassessmentanalyzer.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include <QDate>
#include <QDebug>
#include <QColor>
#include <QtMath>

HealthAssessmentAnalyzer::HealthAssessmentAnalyzer(QObject *parent)
    : QObject(parent)
{
}

HealthAssessmentResult HealthAssessmentAnalyzer::assessPipeline(const Pipeline &pipeline)
{
    HealthAssessmentResult result;
    QMap<QString, double> factors;
    
    // 1. 建设年限得分 (权重: 30%)
    double ageScore = calculateAgeScore(pipeline.buildDate(), 
                                        getMaterialDesignLife(pipeline.material()));
    factors["建设年限"] = ageScore;
    
    // 2. 材质得分 (权重: 25%)
    double materialScore = calculateMaterialScore(pipeline.material());
    factors["材质类型"] = materialScore;
    
    // 3. 巡检记录得分 (权重: 20%)
    double inspectionScore = calculateInspectionScore(pipeline.lastInspection(), 
                                                     pipeline.inspectionCycle());
    factors["巡检记录"] = inspectionScore;
    
    // 4. 状态得分 (权重: 15%)
    double statusScore = calculateStatusScore(pipeline.status());
    factors["运行状态"] = statusScore;
    
    // 5. 当前健康度得分 (权重: 10%) - 如果已有健康度，作为参考
    double currentHealthScore = pipeline.healthScore();
    factors["当前健康度"] = currentHealthScore;
    
    // 计算综合得分
    result.score = calculateFinalScore(factors);
    result.level = getHealthLevel(result.score);
    result.factors = factors;
    
    // 生成评估描述
    QStringList descriptions;
    if (ageScore < 60) {
        descriptions << QString("建设年限较长（%1年），存在老化风险")
                         .arg(pipeline.buildDate().daysTo(QDate::currentDate()) / 365);
    }
    if (materialScore < 70) {
        descriptions << QString("材质类型（%1）耐久性一般").arg(pipeline.material());
    }
    if (inspectionScore < 60) {
        descriptions << "巡检记录不完整或超期";
    }
    if (statusScore < 80) {
        descriptions << QString("运行状态：%1").arg(pipeline.status());
    }
    
    if (descriptions.isEmpty()) {
        result.description = "各项指标良好，健康度优秀";
    } else {
        result.description = descriptions.join("；");
    }
    
    return result;
}

HealthAssessmentResult HealthAssessmentAnalyzer::assessFacility(const Facility &facility)
{
    HealthAssessmentResult result;
    QMap<QString, double> factors;
    
    // 1. 建设年限得分 (权重: 30%)
    double ageScore = calculateAgeScore(facility.buildDate(), 
                                        getMaterialDesignLife(facility.material()));
    factors["建设年限"] = ageScore;
    
    // 2. 材质得分 (权重: 25%)
    double materialScore = calculateMaterialScore(facility.material());
    factors["材质类型"] = materialScore;
    
    // 3. 维护记录得分 (权重: 25%)
    QDate lastMaintenance = facility.lastMaintenance();
    int maintenanceCycle = 365; // 默认1年
    if (!facility.nextMaintenance().isNull() && !lastMaintenance.isNull()) {
        maintenanceCycle = lastMaintenance.daysTo(facility.nextMaintenance());
    }
    double maintenanceScore = calculateInspectionScore(lastMaintenance, maintenanceCycle);
    factors["维护记录"] = maintenanceScore;
    
    // 4. 状态得分 (权重: 15%)
    double statusScore = calculateStatusScore(facility.status());
    factors["运行状态"] = statusScore;
    
    // 5. 当前健康度得分 (权重: 5%)
    double currentHealthScore = facility.healthScore();
    factors["当前健康度"] = currentHealthScore;
    
    // 计算综合得分
    result.score = calculateFinalScore(factors);
    result.level = getHealthLevel(result.score);
    result.factors = factors;
    
    // 生成评估描述
    QStringList descriptions;
    if (ageScore < 60) {
        descriptions << QString("建设年限较长（%1年），存在老化风险")
                         .arg(facility.buildDate().daysTo(QDate::currentDate()) / 365);
    }
    if (materialScore < 70) {
        descriptions << QString("材质类型（%1）耐久性一般").arg(facility.material());
    }
    if (maintenanceScore < 60) {
        descriptions << "维护记录不完整或超期";
    }
    if (statusScore < 80) {
        descriptions << QString("运行状态：%1").arg(facility.status());
    }
    
    if (descriptions.isEmpty()) {
        result.description = "各项指标良好，健康度优秀";
    } else {
        result.description = descriptions.join("；");
    }
    
    return result;
}

QMap<QString, int> HealthAssessmentAnalyzer::batchAssessPipelines()
{
    QMap<QString, int> statistics;
    statistics["优秀"] = 0;
    statistics["良好"] = 0;
    statistics["一般"] = 0;
    statistics["较差"] = 0;
    statistics["危险"] = 0;
    
    PipelineDAO dao;
    // 查询所有类型的管线（因为 BaseDAO::findAll 不包含 ST_AsText，需要分别查询）
    QVector<Pipeline> pipelines;
    QStringList types = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
    for (const QString &type : types) {
        QVector<Pipeline> typePipelines = dao.findByType(type, 10000);
        pipelines.append(typePipelines);
    }
    
    int total = pipelines.size();
    int current = 0;
    
    for (const Pipeline &pipeline : pipelines) {
        HealthAssessmentResult result = assessPipeline(pipeline);
        
        // 更新健康度分数到数据库
        Pipeline updatedPipeline = pipeline;
        updatedPipeline.setHealthScore(result.score);
        dao.update(updatedPipeline, pipeline.id());
        
        // 统计
        statistics[result.level]++;
        
        current++;
        emit assessmentProgress(current, total);
    }
    
    emit assessmentComplete(total, 0);
    return statistics;
}

QMap<QString, int> HealthAssessmentAnalyzer::batchAssessFacilities()
{
    QMap<QString, int> statistics;
    statistics["优秀"] = 0;
    statistics["良好"] = 0;
    statistics["一般"] = 0;
    statistics["较差"] = 0;
    statistics["危险"] = 0;
    
    FacilityDAO dao;
    QVector<Facility> facilities = dao.findAll(10000); // 查询所有设施
    
    int total = facilities.size();
    int current = 0;
    
    for (const Facility &facility : facilities) {
        HealthAssessmentResult result = assessFacility(facility);
        
        // 更新健康度分数到数据库
        Facility updatedFacility = facility;
        updatedFacility.setHealthScore(result.score);
        dao.update(updatedFacility, facility.id());
        
        // 统计
        statistics[result.level]++;
        
        current++;
        emit assessmentProgress(current, total);
    }
    
    emit assessmentComplete(0, total);
    return statistics;
}

QString HealthAssessmentAnalyzer::getHealthLevel(int score)
{
    if (score >= 90) {
        return "优秀";
    } else if (score >= 75) {
        return "良好";
    } else if (score >= 60) {
        return "一般";
    } else if (score >= 40) {
        return "较差";
    } else {
        return "危险";
    }
}

QColor HealthAssessmentAnalyzer::getHealthColor(int score)
{
    if (score >= 90) {
        return QColor(0, 200, 0);      // 绿色 - 优秀
    } else if (score >= 75) {
        return QColor(100, 200, 100);  // 浅绿色 - 良好
    } else if (score >= 60) {
        return QColor(255, 200, 0);    // 黄色 - 一般
    } else if (score >= 40) {
        return QColor(255, 150, 0);    // 橙色 - 较差
    } else {
        return QColor(255, 0, 0);      // 红色 - 危险
    }
}

double HealthAssessmentAnalyzer::calculateAgeScore(const QDate &buildDate, int designLife) const
{
    if (!buildDate.isValid()) {
        return 80.0; // 如果没有建设日期，给中等分数
    }
    
    int age = buildDate.daysTo(QDate::currentDate()) / 365; // 年龄（年）
    
    if (age < 0) {
        return 100.0; // 未来日期，给满分
    }
    
    double lifeRatio = static_cast<double>(age) / designLife;
    
    if (lifeRatio < 0.3) {
        return 100.0; // 使用年限 < 30%，优秀
    } else if (lifeRatio < 0.5) {
        return 90.0 - (lifeRatio - 0.3) * 50.0; // 30%-50%，90-80分
    } else if (lifeRatio < 0.7) {
        return 80.0 - (lifeRatio - 0.5) * 50.0; // 50%-70%，80-70分
    } else if (lifeRatio < 0.9) {
        return 70.0 - (lifeRatio - 0.7) * 100.0; // 70%-90%，70-50分
    } else if (lifeRatio < 1.0) {
        return 50.0 - (lifeRatio - 0.9) * 200.0; // 90%-100%，50-30分
    } else {
        return qMax(10.0, 30.0 - (lifeRatio - 1.0) * 20.0); // 超过设计寿命，30-10分
    }
}

double HealthAssessmentAnalyzer::calculateMaterialScore(const QString &material) const
{
    // 材质耐久性评分
    QMap<QString, double> materialScores = {
        {"不锈钢", 100.0},
        {"球墨铸铁", 95.0},
        {"PE", 90.0},
        {"PVC", 85.0},
        {"HDPE", 90.0},
        {"钢管", 80.0},
        {"铸铁", 75.0},
        {"混凝土", 70.0},
        {"石棉", 50.0},  // 老旧材质，不推荐
        {"陶瓷", 65.0}
    };
    
    QString materialLower = material.toLower();
    for (auto it = materialScores.begin(); it != materialScores.end(); ++it) {
        if (materialLower.contains(it.key().toLower())) {
            return it.value();
        }
    }
    
    return 75.0; // 默认中等分数
}

double HealthAssessmentAnalyzer::calculateInspectionScore(const QDate &lastInspection, int inspectionCycle) const
{
    if (!lastInspection.isValid() || inspectionCycle <= 0) {
        return 50.0; // 没有巡检记录，给低分
    }
    
    int daysSinceInspection = lastInspection.daysTo(QDate::currentDate());
    
    if (daysSinceInspection < 0) {
        return 100.0; // 未来日期，满分
    }
    
    double cycleRatio = static_cast<double>(daysSinceInspection) / inspectionCycle;
    
    if (cycleRatio < 0.5) {
        return 100.0; // 巡检及时
    } else if (cycleRatio < 1.0) {
        return 90.0 - (cycleRatio - 0.5) * 40.0; // 50%-100%，90-70分
    } else if (cycleRatio < 1.5) {
        return 70.0 - (cycleRatio - 1.0) * 40.0; // 100%-150%，70-50分
    } else if (cycleRatio < 2.0) {
        return 50.0 - (cycleRatio - 1.5) * 40.0; // 150%-200%，50-30分
    } else {
        return qMax(10.0, 30.0 - (cycleRatio - 2.0) * 10.0); // 超过2个周期，30-10分
    }
}

double HealthAssessmentAnalyzer::calculateStatusScore(const QString &status) const
{
    QString statusLower = status.toLower();
    
    if (statusLower.contains("正常") || statusLower.contains("active") || 
        statusLower.contains("运行") || statusLower == "normal") {
        return 100.0;
    } else if (statusLower.contains("维护") || statusLower.contains("maintenance")) {
        return 80.0;
    } else if (statusLower.contains("停用") || statusLower.contains("停运") || 
               statusLower.contains("disabled")) {
        return 40.0;
    } else if (statusLower.contains("故障") || statusLower.contains("故障") || 
               statusLower.contains("fault")) {
        return 20.0;
    } else {
        return 70.0; // 未知状态，给中等分数
    }
}

int HealthAssessmentAnalyzer::getMaterialDesignLife(const QString &material) const
{
    // 不同材质的设计寿命（年）
    QMap<QString, int> designLives = {
        {"不锈钢", 50},
        {"球墨铸铁", 50},
        {"PE", 50},
        {"HDPE", 50},
        {"PVC", 40},
        {"钢管", 30},
        {"铸铁", 40},
        {"混凝土", 50},
        {"石棉", 30},
        {"陶瓷", 40}
    };
    
    QString materialLower = material.toLower();
    for (auto it = designLives.begin(); it != designLives.end(); ++it) {
        if (materialLower.contains(it.key().toLower())) {
            return it.value();
        }
    }
    
    return 40; // 默认40年
}

int HealthAssessmentAnalyzer::calculateFinalScore(const QMap<QString, double> &factors) const
{
    // 权重配置
    QMap<QString, double> weights;
    
    if (factors.contains("建设年限")) {
        weights["建设年限"] = 0.30;
    }
    if (factors.contains("材质类型")) {
        weights["材质类型"] = 0.25;
    }
    if (factors.contains("巡检记录") || factors.contains("维护记录")) {
        weights[factors.contains("巡检记录") ? "巡检记录" : "维护记录"] = 0.20;
    }
    if (factors.contains("运行状态")) {
        weights["运行状态"] = 0.15;
    }
    if (factors.contains("当前健康度")) {
        weights["当前健康度"] = factors.contains("巡检记录") ? 0.10 : 0.05;
    }
    
    // 计算加权平均分
    double totalScore = 0.0;
    double totalWeight = 0.0;
    
    for (auto it = weights.begin(); it != weights.end(); ++it) {
        QString factorName = it.key();
        double weight = it.value();
        double score = factors.value(factorName, 0.0);
        
        totalScore += score * weight;
        totalWeight += weight;
    }
    
    // 归一化（如果权重总和不为1）
    if (totalWeight > 0) {
        totalScore = totalScore / totalWeight;
    }
    
    // 四舍五入到整数
    return qBound(0, static_cast<int>(qRound(totalScore)), 100);
}

