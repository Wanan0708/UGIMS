#include "widgets/drawingtoolpanel.h"
#include <QScrollArea>
#include <QLabel>

DrawingToolPanel::DrawingToolPanel(QWidget *parent)
    : QWidget(parent)
    , m_currentType(None)
{
    setupUI();
    setupConnections();
}

DrawingToolPanel::~DrawingToolPanel()
{
}

void DrawingToolPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(6);
    
    // ========== 管线工具组 ==========
    m_pipelineGroup = new CollapsibleGroupBox("📍 管线类型", this);
    
    QVBoxLayout *pipelineLayout = new QVBoxLayout();
    pipelineLayout->setSpacing(4);
    pipelineLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建管线按钮
    m_waterSupplyBtn = createToolButton("💧 给水管");
    m_sewageBtn = createToolButton("🚰 排水管");
    m_gasBtn = createToolButton("🔥 燃气管");
    m_electricBtn = createToolButton("⚡ 电力电缆");
    m_telecomBtn = createToolButton("📡 通信光缆");
    m_heatBtn = createToolButton("🌡️ 供热管");
    
    pipelineLayout->addWidget(m_waterSupplyBtn);
    pipelineLayout->addWidget(m_sewageBtn);
    pipelineLayout->addWidget(m_gasBtn);
    pipelineLayout->addWidget(m_electricBtn);
    pipelineLayout->addWidget(m_telecomBtn);
    pipelineLayout->addWidget(m_heatBtn);
    
    // 设置管线组内容
    m_pipelineGroup->setContentLayout(pipelineLayout);
    m_pipelineGroup->setExpanded(true, false);  // 默认展开，不用动画
    
    // 管线按钮组
    m_pipelineButtonGroup = new QButtonGroup(this);
    m_pipelineButtonGroup->addButton(m_waterSupplyBtn, WaterSupply);
    m_pipelineButtonGroup->addButton(m_sewageBtn, Sewage);
    m_pipelineButtonGroup->addButton(m_gasBtn, Gas);
    m_pipelineButtonGroup->addButton(m_electricBtn, Electric);
    m_pipelineButtonGroup->addButton(m_telecomBtn, Telecom);
    m_pipelineButtonGroup->addButton(m_heatBtn, Heat);
    m_pipelineButtonGroup->setExclusive(true);
    
    // ========== 设施工具组 ==========
    m_facilityGroup = new CollapsibleGroupBox("🔧 设施类型", this);
    
    QVBoxLayout *facilityLayout = new QVBoxLayout();
    facilityLayout->setSpacing(4);
    facilityLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建设施按钮
    m_valveBtn = createToolButton("🔵 阀门");
    m_manholeBtn = createToolButton("🟢 井盖");
    m_pumpStationBtn = createToolButton("🏗️ 泵站");
    m_transformerBtn = createToolButton("🔌 变压器");
    m_regulatorBtn = createToolButton("⚙️ 调压站");
    m_junctionBoxBtn = createToolButton("📦 接线盒");
    
    facilityLayout->addWidget(m_valveBtn);
    facilityLayout->addWidget(m_manholeBtn);
    facilityLayout->addWidget(m_pumpStationBtn);
    facilityLayout->addWidget(m_transformerBtn);
    facilityLayout->addWidget(m_regulatorBtn);
    facilityLayout->addWidget(m_junctionBoxBtn);
    
    // 设置设施组内容
    m_facilityGroup->setContentLayout(facilityLayout);
    m_facilityGroup->setExpanded(true, false);  // 默认展开，不用动画
    
    // 设施按钮组
    m_facilityButtonGroup = new QButtonGroup(this);
    m_facilityButtonGroup->addButton(m_valveBtn, Valve);
    m_facilityButtonGroup->addButton(m_manholeBtn, Manhole);
    m_facilityButtonGroup->addButton(m_pumpStationBtn, PumpStation);
    m_facilityButtonGroup->addButton(m_transformerBtn, Transformer);
    m_facilityButtonGroup->addButton(m_regulatorBtn, Regulator);
    m_facilityButtonGroup->addButton(m_junctionBoxBtn, JunctionBox);
    m_facilityButtonGroup->setExclusive(true);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_pipelineGroup);
    m_mainLayout->addWidget(m_facilityGroup);
    m_mainLayout->addStretch();
}

void DrawingToolPanel::setupConnections()
{
    connect(m_pipelineButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DrawingToolPanel::onPipelineButtonClicked);
    
    connect(m_facilityButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DrawingToolPanel::onFacilityButtonClicked);
}

QPushButton* DrawingToolPanel::createToolButton(const QString &text, const QString &iconPath)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setMinimumHeight(32);
    btn->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  padding-left: 8px;"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #e6f7ff;"
        "  border-color: #40a9ff;"
        "}"
        "QPushButton:checked {"
        "  background-color: #1890ff;"
        "  color: white;"
        "  border-color: #1890ff;"
        "  font-weight: bold;"
        "}"
    );
    
    // 如果提供了图标路径，设置图标
    if (!iconPath.isEmpty()) {
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(20, 20));
    }
    
    return btn;
}

void DrawingToolPanel::onPipelineButtonClicked(int id)
{
    // 取消设施组的选择
    if (m_facilityButtonGroup->checkedButton()) {
        m_facilityButtonGroup->setExclusive(false);
        m_facilityButtonGroup->checkedButton()->setChecked(false);
        m_facilityButtonGroup->setExclusive(true);
    }
    
    m_currentType = static_cast<DrawingType>(id);
    
    emit drawingTypeChanged(m_currentType);
    emit startDrawingPipeline(currentTypeId());
}

void DrawingToolPanel::onFacilityButtonClicked(int id)
{
    // 取消管线组的选择
    if (m_pipelineButtonGroup->checkedButton()) {
        m_pipelineButtonGroup->setExclusive(false);
        m_pipelineButtonGroup->checkedButton()->setChecked(false);
        m_pipelineButtonGroup->setExclusive(true);
    }
    
    m_currentType = static_cast<DrawingType>(id);
    
    emit drawingTypeChanged(m_currentType);
    emit startDrawingFacility(currentTypeId());
}

DrawingToolPanel::DrawingType DrawingToolPanel::currentDrawingType() const
{
    return m_currentType;
}

QString DrawingToolPanel::currentTypeId() const
{
    static QMap<DrawingType, QString> typeIdMap = {
        {WaterSupply, "water_supply"},
        {Sewage, "sewage"},
        {Gas, "gas"},
        {Electric, "electric"},
        {Telecom, "telecom"},
        {Heat, "heat"},
        {Valve, "valve"},
        {Manhole, "manhole"},
        {PumpStation, "pump_station"},
        {Transformer, "transformer"},
        {Regulator, "regulator"},
        {JunctionBox, "junction_box"}
    };
    
    return typeIdMap.value(m_currentType, "");
}

QString DrawingToolPanel::currentTypeName() const
{
    static QMap<DrawingType, QString> typeNameMap = {
        {WaterSupply, "给水管"},
        {Sewage, "排水管"},
        {Gas, "燃气管"},
        {Electric, "电力电缆"},
        {Telecom, "通信光缆"},
        {Heat, "供热管"},
        {Valve, "阀门"},
        {Manhole, "井盖"},
        {PumpStation, "泵站"},
        {Transformer, "变压器"},
        {Regulator, "调压站"},
        {JunctionBox, "接线盒"}
    };
    
    return typeNameMap.value(m_currentType, "未知");
}

bool DrawingToolPanel::isPipelineType() const
{
    return m_currentType >= WaterSupply && m_currentType <= Heat;
}

bool DrawingToolPanel::isFacilityType() const
{
    return m_currentType >= Valve && m_currentType <= JunctionBox;
}

void DrawingToolPanel::resetSelection()
{
    // 取消所有选择
    if (m_pipelineButtonGroup->checkedButton()) {
        m_pipelineButtonGroup->setExclusive(false);
        m_pipelineButtonGroup->checkedButton()->setChecked(false);
        m_pipelineButtonGroup->setExclusive(true);
    }
    
    if (m_facilityButtonGroup->checkedButton()) {
        m_facilityButtonGroup->setExclusive(false);
        m_facilityButtonGroup->checkedButton()->setChecked(false);
        m_facilityButtonGroup->setExclusive(true);
    }
    
    m_currentType = None;
    emit drawingTypeChanged(None);
}
