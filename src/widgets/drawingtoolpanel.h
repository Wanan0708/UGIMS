#ifndef DRAWINGTOOLPANEL_H
#define DRAWINGTOOLPANEL_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include "widgets/collapsiblegroupbox.h"

/**
 * @brief 绘制工具面板
 * 提供管线和设施的绘制工具选择
 */
class DrawingToolPanel : public QWidget
{
    Q_OBJECT

public:
    // 绘制类型枚举
    enum DrawingType {
        None = 0,           // 无选择
        
        // 管线类型
        WaterSupply,        // 给水管
        Sewage,             // 排水管
        Gas,                // 燃气管
        Electric,           // 电力电缆
        Telecom,            // 通信光缆
        Heat,               // 供热管
        
        // 设施类型
        Valve,              // 阀门
        Manhole,            // 井盖
        PumpStation,        // 泵站
        Transformer,        // 变压器
        Regulator,          // 调压站
        JunctionBox         // 接线盒
    };
    Q_ENUM(DrawingType)

    explicit DrawingToolPanel(QWidget *parent = nullptr);
    ~DrawingToolPanel();

    // 获取当前选中的绘制类型
    DrawingType currentDrawingType() const;
    
    // 获取当前选中类型的字符串标识
    QString currentTypeId() const;
    
    // 获取当前选中类型的显示名称
    QString currentTypeName() const;
    
    // 判断当前选择是否为管线
    bool isPipelineType() const;
    
    // 判断当前选择是否为设施
    bool isFacilityType() const;
    
    // 重置选择
    void resetSelection();
    
    // 获取当前线宽
    int currentLineWidth() const { return m_lineWidthSpin ? m_lineWidthSpin->value() : 3; }
    
    // 获取当前颜色（返回颜色名称）
    QString currentColorName() const;
    
    // 获取当前颜色（返回QColor）
    QColor currentColor() const;

signals:
    // 绘制类型改变信号
    void drawingTypeChanged(DrawingType type);
    
    // 开始绘制管线信号
    void startDrawingPipeline(const QString &pipelineType);
    
    // 开始绘制设施信号
    void startDrawingFacility(const QString &facilityType);

private slots:
    void onPipelineButtonClicked(int id);
    void onFacilityButtonClicked(int id);

private:
    void setupUI();
    void setupConnections();
    QPushButton* createToolButton(const QString &text, const QString &iconPath = QString());

private:
    // 当前绘制类型
    DrawingType m_currentType;
    
    // 管线工具按钮组
    QButtonGroup *m_pipelineButtonGroup;
    QPushButton *m_waterSupplyBtn;
    QPushButton *m_sewageBtn;
    QPushButton *m_gasBtn;
    QPushButton *m_electricBtn;
    QPushButton *m_telecomBtn;
    QPushButton *m_heatBtn;
    
    // 设施工具按钮组
    QButtonGroup *m_facilityButtonGroup;
    QPushButton *m_valveBtn;
    QPushButton *m_manholeBtn;
    QPushButton *m_pumpStationBtn;
    QPushButton *m_transformerBtn;
    QPushButton *m_regulatorBtn;
    QPushButton *m_junctionBoxBtn;
    
    // 布局
    QVBoxLayout *m_mainLayout;
    CollapsibleGroupBox *m_pipelineGroup;
    CollapsibleGroupBox *m_facilityGroup;
    CollapsibleGroupBox *m_styleGroup;  // 样式设置组
    
    // 样式设置控件
    QComboBox *m_colorCombo;       // 颜色选择
    QSpinBox *m_lineWidthSpin;     // 线宽设置
};

#endif // DRAWINGTOOLPANEL_H
