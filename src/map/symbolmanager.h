#ifndef SYMBOLMANAGER_H
#define SYMBOLMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QHash>

/**
 * @brief 符号管理器
 * 管理管线和设施的颜色、样式等符号化配置
 */
class SymbolManager : public QObject
{
    Q_OBJECT

public:
    explicit SymbolManager(QObject *parent = nullptr);

    // 获取管线颜色
    QColor getPipelineColor(const QString &pipelineType) const;
    
    // 获取管线画笔
    QPen getPipelinePen(const QString &pipelineType, int diameterMm = 0) const;
    
    // 获取设施颜色
    QColor getFacilityColor(const QString &facilityType) const;
    
    // 获取设施画刷
    QBrush getFacilityBrush(const QString &facilityType) const;
    
    // 根据管径计算线宽
    qreal calculateLineWidth(int diameterMm) const;
    
    // 根据设施类型获取图标大小
    int getFacilityIconSize(const QString &facilityType) const;
    
    // 获取健康度颜色
    QColor getHealthScoreColor(int healthScore) const;
    
    // 设置自定义颜色
    void setPipelineColor(const QString &pipelineType, const QColor &color);
    void setFacilityColor(const QString &facilityType, const QColor &color);
    
    // 从配置加载颜色方案
    void loadColorScheme();

private:
    // 管线类型颜色映射
    QHash<QString, QColor> m_pipelineColors;
    
    // 设施类型颜色映射
    QHash<QString, QColor> m_facilityColors;
    
    // 初始化默认颜色
    void initializeDefaultColors();
};

#endif // SYMBOLMANAGER_H

