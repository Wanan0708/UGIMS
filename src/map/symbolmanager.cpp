#include "map/symbolmanager.h"
#include "core/common/config.h"
#include "core/common/logger.h"
#include <QtMath>

SymbolManager::SymbolManager(QObject *parent)
    : QObject(parent)
{
    initializeDefaultColors();
    loadColorScheme();
}

void SymbolManager::initializeDefaultColors()
{
    // 管线默认颜色（参考行业标准）
    m_pipelineColors["water_supply"] = QColor("#0066CC");    // 蓝色 - 给水
    m_pipelineColors["sewage"] = QColor("#8B4513");          // 棕色 - 排水
    m_pipelineColors["gas"] = QColor("#FFD700");             // 金黄 - 燃气
    m_pipelineColors["electric"] = QColor("#FF0000");        // 红色 - 电力
    m_pipelineColors["telecom"] = QColor("#00FF00");         // 绿色 - 通信
    m_pipelineColors["heat"] = QColor("#FF8C00");            // 橙色 - 供热
    
    // 设施默认颜色
    m_facilityColors["valve"] = QColor("#4169E1");           // 皇家蓝 - 阀门
    m_facilityColors["manhole"] = QColor("#696969");         // 深灰 - 井盖
    m_facilityColors["pump_station"] = QColor("#1E90FF");    // 道奇蓝 - 泵站
    m_facilityColors["transformer"] = QColor("#DC143C");     // 猩红 - 变压器
    m_facilityColors["regulator"] = QColor("#FFD700");       // 金色 - 调压站
    m_facilityColors["junction_box"] = QColor("#32CD32");    // 草绿 - 接线盒
}

void SymbolManager::loadColorScheme()
{
    // 从配置文件加载自定义颜色
    Config &config = Config::instance();
    
    // 加载管线颜色
    QString waterColor = config.getString("Pipeline/color_water");
    if (!waterColor.isEmpty()) {
        m_pipelineColors["water_supply"] = QColor(waterColor);
    }
    
    QString gasColor = config.getString("Pipeline/color_gas");
    if (!gasColor.isEmpty()) {
        m_pipelineColors["gas"] = QColor(gasColor);
    }
    
    QString electricColor = config.getString("Pipeline/color_electric");
    if (!electricColor.isEmpty()) {
        m_pipelineColors["electric"] = QColor(electricColor);
    }
    
    QString telecomColor = config.getString("Pipeline/color_telecom");
    if (!telecomColor.isEmpty()) {
        m_pipelineColors["telecom"] = QColor(telecomColor);
    }
    
    QString heatColor = config.getString("Pipeline/color_heat");
    if (!heatColor.isEmpty()) {
        m_pipelineColors["heat"] = QColor(heatColor);
    }
    
    QString sewageColor = config.getString("Pipeline/color_sewage");
    if (!sewageColor.isEmpty()) {
        m_pipelineColors["sewage"] = QColor(sewageColor);
    }
    
    LOG_INFO("Color scheme loaded");
}

QColor SymbolManager::getPipelineColor(const QString &pipelineType) const
{
    return m_pipelineColors.value(pipelineType, QColor("#808080"));  // 默认灰色
}

QPen SymbolManager::getPipelinePen(const QString &pipelineType, int diameterMm) const
{
    QColor color = getPipelineColor(pipelineType);
    qreal width = calculateLineWidth(diameterMm);
    
    QPen pen(color);
    pen.setWidthF(width);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    
    // 燃气管线使用虚线
    if (pipelineType == "gas") {
        pen.setStyle(Qt::DashLine);
    }
    
    return pen;
}

QColor SymbolManager::getFacilityColor(const QString &facilityType) const
{
    return m_facilityColors.value(facilityType, QColor("#808080"));  // 默认灰色
}

QBrush SymbolManager::getFacilityBrush(const QString &facilityType) const
{
    QColor color = getFacilityColor(facilityType);
    return QBrush(color);
}

qreal SymbolManager::calculateLineWidth(int diameterMm) const
{
    // 根据管径计算显示宽度
    // 使用对数刻度使小管径和大管径都能合理显示
    
    if (diameterMm <= 0) {
        return 3.0;  // 默认宽度
    }
    
    // 基础宽度 + 管径影响
    // DN100 -> 2.5, DN200 -> 3.5, DN400 -> 4.5, DN800 -> 5.5
    qreal width = 2.0 + qLn(diameterMm / 50.0);
    
    // 限制范围
    width = qBound(2.0, width, 8.0);
    
    return width;
}

int SymbolManager::getFacilityIconSize(const QString &facilityType) const
{
    // 根据设施类型返回不同的图标大小
    if (facilityType == "pump_station" || facilityType == "transformer") {
        return 20;  // 大型设施
    } else if (facilityType == "valve" || facilityType == "regulator") {
        return 16;  // 中型设施
    } else {
        return 12;  // 小型设施（井盖等）
    }
}

QColor SymbolManager::getHealthScoreColor(int healthScore) const
{
    // 根据健康度返回颜色
    // 90-100: 绿色（优秀）
    // 70-89:  黄色（良好）
    // 50-69:  橙色（一般）
    // 0-49:   红色（差）
    
    if (healthScore >= 90) {
        return QColor("#00FF00");  // 绿色
    } else if (healthScore >= 70) {
        return QColor("#FFFF00");  // 黄色
    } else if (healthScore >= 50) {
        return QColor("#FFA500");  // 橙色
    } else {
        return QColor("#FF0000");  // 红色
    }
}

void SymbolManager::setPipelineColor(const QString &pipelineType, const QColor &color)
{
    m_pipelineColors[pipelineType] = color;
    LOG_INFO(QString("Pipeline color set: %1 -> %2")
                 .arg(pipelineType, color.name()));
}

void SymbolManager::setFacilityColor(const QString &facilityType, const QColor &color)
{
    m_facilityColors[facilityType] = color;
    LOG_INFO(QString("Facility color set: %1 -> %2")
                 .arg(facilityType, color.name()));
}

