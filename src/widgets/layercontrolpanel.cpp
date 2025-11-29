#include "layercontrolpanel.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>
#include <QTimer>
#include <QDebug>

LayerControlPanel::LayerControlPanel(QWidget *parent)
    : QWidget(parent)
    , m_layerManager(nullptr)
    , m_waterPipelineCheck(nullptr)
    , m_sewagePipelineCheck(nullptr)
    , m_gasPipelineCheck(nullptr)
    , m_electricPipelineCheck(nullptr)
    , m_telecomPipelineCheck(nullptr)
    , m_heatPipelineCheck(nullptr)
    , m_facilitiesCheck(nullptr)
    , m_labelsCheck(nullptr)
{
    setupUI();
    setupConnections();
}

LayerControlPanel::~LayerControlPanel()
{
}

void LayerControlPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);
    
    // ========== 管线图层组 ==========
    m_pipelineGroup = new CollapsibleGroupBox("🗺️ 管线图层", this);
    m_pipelineGroup->setExpanded(true);
    
    QVBoxLayout *pipelineLayout = new QVBoxLayout();
    pipelineLayout->setSpacing(4);
    pipelineLayout->setContentsMargins(8, 8, 8, 8);
    
    // 创建管线图层控制项
    pipelineLayout->addWidget(createLayerItem(LayerManager::WaterPipeline, "给水管线", QColor(0, 112, 192)));
    pipelineLayout->addWidget(createLayerItem(LayerManager::SewagePipeline, "排水管线", QColor(112, 48, 160)));
    pipelineLayout->addWidget(createLayerItem(LayerManager::GasPipeline, "燃气管线", QColor(255, 192, 0)));
    pipelineLayout->addWidget(createLayerItem(LayerManager::ElectricPipeline, "电力电缆", QColor(255, 0, 0)));
    pipelineLayout->addWidget(createLayerItem(LayerManager::TelecomPipeline, "通信光缆", QColor(0, 176, 80)));
    pipelineLayout->addWidget(createLayerItem(LayerManager::HeatPipeline, "供热管线", QColor(255, 128, 0)));
    
    m_pipelineGroup->setContentLayout(pipelineLayout);
    m_mainLayout->addWidget(m_pipelineGroup);
    
    // ========== 设施图层组 ==========
    m_facilitiesGroup = new CollapsibleGroupBox("📍 设施图层", this);
    m_facilitiesGroup->setExpanded(true);
    
    QVBoxLayout *facilitiesLayout = new QVBoxLayout();
    facilitiesLayout->setSpacing(4);
    facilitiesLayout->setContentsMargins(8, 8, 8, 8);
    
    facilitiesLayout->addWidget(createLayerItem(LayerManager::Facilities, "设施点", QColor(255, 122, 24)));
    
    m_facilitiesGroup->setContentLayout(facilitiesLayout);
    m_mainLayout->addWidget(m_facilitiesGroup);
    
    // ========== 其他图层组 ==========
    m_otherGroup = new CollapsibleGroupBox("🏷️ 其他图层", this);
    m_otherGroup->setExpanded(false);
    
    QVBoxLayout *otherLayout = new QVBoxLayout();
    otherLayout->setSpacing(4);
    otherLayout->setContentsMargins(8, 8, 8, 8);
    
    otherLayout->addWidget(createLayerItem(LayerManager::Labels, "标注", QColor(64, 64, 64)));
    
    m_otherGroup->setContentLayout(otherLayout);
    m_mainLayout->addWidget(m_otherGroup);
    
    // ========== 分隔线 ==========
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("QFrame { color: #e0e0e0; }");
    m_mainLayout->addWidget(separator);
    
    // ========== 刷新按钮 ==========
    m_refreshAllBtn = new QPushButton("🔄 刷新所有图层");
    m_refreshAllBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4CAF50, stop:1 #45a049);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5BC85F, stop:1 #4CAF50);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #45a049, stop:1 #3d8b40);"
        "}"
    );
    m_mainLayout->addWidget(m_refreshAllBtn);
    
    // 添加弹簧
    m_mainLayout->addStretch();
    
    // 设置面板样式
    setStyleSheet(
        "LayerControlPanel {"
        "  background: white;"
        "  border-radius: 4px;"
        "}"
    );
}

QWidget* LayerControlPanel::createLayerItem(LayerManager::LayerType type, 
                                             const QString &name, 
                                             const QColor &color)
{
    QWidget *item = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);
    
    // 颜色指示器
    QLabel *colorIndicator = new QLabel();
    colorIndicator->setFixedSize(16, 16);
    colorIndicator->setStyleSheet(QString(
        "QLabel {"
        "  background-color: %1;"
        "  border: 1px solid #ccc;"
        "  border-radius: 3px;"
        "}"
    ).arg(color.name()));
    layout->addWidget(colorIndicator);
    
    // 复选框
    QCheckBox *checkBox = new QCheckBox(name);
    checkBox->setChecked(true);  // 默认选中
    checkBox->setStyleSheet(
        "QCheckBox {"
        "  font-size: 13px;"
        "  spacing: 6px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "  border-radius: 3px;"
        "  border: 2px solid #ccc;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #4CAF50;"
        "  border-color: #4CAF50;"
        "  image: url(:/icons/check.png);"  // 可选：添加对勾图标
        "}"
        "QCheckBox::indicator:unchecked {"
        "  background-color: white;"
        "}"
        "QCheckBox::indicator:hover {"
        "  border-color: #4CAF50;"
        "}"
    );
    layout->addWidget(checkBox, 1);
    
    // 保存映射关系
    m_checkBoxMap[checkBox] = type;
    
    // 根据类型保存引用
    switch (type) {
    case LayerManager::WaterPipeline:
        m_waterPipelineCheck = checkBox;
        break;
    case LayerManager::SewagePipeline:
        m_sewagePipelineCheck = checkBox;
        break;
    case LayerManager::GasPipeline:
        m_gasPipelineCheck = checkBox;
        break;
    case LayerManager::ElectricPipeline:
        m_electricPipelineCheck = checkBox;
        break;
    case LayerManager::TelecomPipeline:
        m_telecomPipelineCheck = checkBox;
        break;
    case LayerManager::HeatPipeline:
        m_heatPipelineCheck = checkBox;
        break;
    case LayerManager::Facilities:
        m_facilitiesCheck = checkBox;
        break;
    case LayerManager::Labels:
        m_labelsCheck = checkBox;
        break;
    default:
        break;
    }
    
    // 设置item样式
    item->setStyleSheet(
        "QWidget {"
        "  background: transparent;"
        "}"
        "QWidget:hover {"
        "  background: rgba(76, 175, 80, 0.05);"
        "  border-radius: 3px;"
        "}"
    );
    
    return item;
}

void LayerControlPanel::setupConnections()
{
    // 连接所有复选框的信号
    for (auto it = m_checkBoxMap.begin(); it != m_checkBoxMap.end(); ++it) {
        QCheckBox *checkBox = it.key();
        connect(checkBox, &QCheckBox::toggled, this, &LayerControlPanel::onLayerCheckBoxToggled);
    }
    
    // 连接刷新按钮
    connect(m_refreshAllBtn, &QPushButton::clicked, this, &LayerControlPanel::onRefreshAllClicked);
}

void LayerControlPanel::setLayerManager(LayerManager *layerManager)
{
    m_layerManager = layerManager;
    
    if (m_layerManager) {
        // 同步当前图层状态
        refresh();
        
        // 连接图层管理器的信号
        connect(m_layerManager, &LayerManager::layerVisibilityChanged,
                this, [this](LayerManager::LayerType type, bool visible) {
            qDebug() << "图层可见性已改变:" << m_layerManager->getLayerName(type) << visible;
        });
    }
}

void LayerControlPanel::refresh()
{
    if (!m_layerManager) {
        return;
    }
    
    // 同步所有复选框的状态
    for (auto it = m_checkBoxMap.begin(); it != m_checkBoxMap.end(); ++it) {
        QCheckBox *checkBox = it.key();
        LayerManager::LayerType type = it.value();
        
        // 阻塞信号，避免触发 toggled 事件
        checkBox->blockSignals(true);
        checkBox->setChecked(m_layerManager->isLayerVisible(type));
        checkBox->blockSignals(false);
    }
    
    qDebug() << "图层控制面板状态已刷新";
}

void LayerControlPanel::onLayerCheckBoxToggled(bool checked)
{
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    if (!checkBox || !m_checkBoxMap.contains(checkBox)) {
        return;
    }
    
    LayerManager::LayerType type = m_checkBoxMap[checkBox];
    
    qDebug() << "图层可见性切换:" << checkBox->text() << checked;
    
    // 通知外部（MyForm）
    emit layerVisibilityChanged(type, checked);
    
    // 如果已设置图层管理器，直接调用
    if (m_layerManager) {
        m_layerManager->setLayerVisible(type, checked);
    }
}

void LayerControlPanel::onRefreshAllClicked()
{
    qDebug() << "刷新所有图层";
    
    if (m_layerManager) {
        m_layerManager->refreshAllLayers();
    }
    
    // 可以添加视觉反馈
    m_refreshAllBtn->setText("✅ 刷新完成");
    QTimer::singleShot(1000, this, [this]() {
        m_refreshAllBtn->setText("🔄 刷新所有图层");
    });
}
