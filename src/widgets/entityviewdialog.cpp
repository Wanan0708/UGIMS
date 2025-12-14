#include "entityviewdialog.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include <QFrame>
#include <QFont>
#include <QGridLayout>
#include <QSpacerItem>
#include <QDate>

EntityViewDialog::EntityViewDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_headerWidget(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_closeBtn(nullptr)
    , m_editBtn(nullptr)
    , m_isPipeline(false)
{
    setWindowTitle("属性详情");
    setMinimumSize(480, 550);
    resize(520, 600);
    setAttribute(Qt::WA_DeleteOnClose);
    
    setupUI();
}

EntityViewDialog::~EntityViewDialog()
{
}

void EntityViewDialog::setupUI()
{
    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ========== 顶部区域（简约标题栏） ==========
    m_headerWidget = new QWidget(this);
    m_headerWidget->setStyleSheet(
        "QWidget {"
        "  background-color: #F8F8F8;"
        "  border-bottom: 1px solid #ddd;"
        "}"
    );
    m_headerWidget->setFixedHeight(50);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setSpacing(10);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // 图标
    QLabel *iconLabel = new QLabel("📊", m_headerWidget);
    QFont iconFont;
    iconFont.setPointSize(20);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(iconLabel);
    
    // 标题
    QLabel *titleLabel = new QLabel("属性详情", m_headerWidget);
    QFont titleFont;
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: #333;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    m_mainLayout->addWidget(m_headerWidget);
    
    // ========== 内容区域（可滚动） ==========
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "  background-color: white;"
        "  border: none;"
        "}"
        "QScrollBar:vertical {"
        "  background: #f0f0f0;"
        "  width: 8px;"
        "  border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #ccc;"
        "  border-radius: 4px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #aaa;"
        "}"
    );
    
    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    // ========== 底部按钮区域 ==========
    QWidget *footerWidget = new QWidget(this);
    footerWidget->setStyleSheet("background-color: #f5f5f5; border-top: 1px solid #ddd;");
    footerWidget->setFixedHeight(50);
    
    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 8, 20, 8);
    footerLayout->addStretch();
    
    m_editBtn = new QPushButton("编辑", footerWidget);
    m_editBtn->setMinimumSize(80, 32);
    m_editBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 3px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0D47A1;"
        "}"
    );
    footerLayout->addWidget(m_editBtn);
    
    m_closeBtn = new QPushButton("关闭", footerWidget);
    m_closeBtn->setMinimumSize(80, 32);
    m_closeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #607D8B;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 3px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #455A64;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #37474F;"
        "}"
    );
    footerLayout->addWidget(m_closeBtn);
    
    m_mainLayout->addWidget(footerWidget);
    
    // 连接信号
    connect(m_closeBtn, &QPushButton::clicked, this, &EntityViewDialog::accept);
    connect(m_editBtn, &QPushButton::clicked, this, &EntityViewDialog::onEditClicked);
}

void EntityViewDialog::setPipeline(const Pipeline &pipeline)
{
    m_isPipeline = true;
    clearContent();
    
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (!contentLayout) return;
    
    // 基本信息卡片
    QWidget *basicCard = createInfoCard("基本信息", "📋");
    addInfoRow(basicCard, "编号", pipeline.pipelineId(), true);
    addInfoRow(basicCard, "名称", pipeline.pipelineName());
    addInfoRow(basicCard, "类型", pipeline.pipelineType());
    addInfoRow(basicCard, "数据库ID", QString::number(pipeline.id()));
    contentLayout->addWidget(basicCard);
    
    // 几何信息卡片
    QWidget *geomCard = createInfoCard("几何信息", "📐");
    addInfoRow(geomCard, "长度", QString("%1 m").arg(pipeline.lengthM(), 0, 'f', 2));
    addInfoRow(geomCard, "埋深", QString("%1 m").arg(pipeline.depthM(), 0, 'f', 2));
    contentLayout->addWidget(geomCard);
    
    // 物理属性卡片
    QWidget *physicalCard = createInfoCard("物理属性", "🔧");
    addInfoRow(physicalCard, "管径", QString("DN%1 mm").arg(pipeline.diameterMm()));
    addInfoRow(physicalCard, "材质", pipeline.material());
    addInfoRow(physicalCard, "压力等级", pipeline.pressureClass());
    contentLayout->addWidget(physicalCard);
    
    // 建设信息卡片
    QWidget *buildCard = createInfoCard("建设信息", "🏗️");
    addInfoRow(buildCard, "建设日期", pipeline.buildDate().isValid() ? 
               pipeline.buildDate().toString("yyyy-MM-dd") : "未设置");
    addInfoRow(buildCard, "施工单位", pipeline.builder());
    addInfoRow(buildCard, "产权单位", pipeline.owner());
    addInfoRow(buildCard, "建设造价", QString("%1 元").arg(pipeline.constructionCost(), 0, 'f', 2));
    contentLayout->addWidget(buildCard);
    
    // 运维信息卡片
    QWidget *maintenanceCard = createInfoCard("运维信息", "⚙️");
    addInfoRow(maintenanceCard, "运行状态", pipeline.status());
    addInfoRow(maintenanceCard, "健康度", QString("%1 分").arg(pipeline.healthScore()), 
               false, pipeline.healthScore() < 60);
    addInfoRow(maintenanceCard, "上次巡检", pipeline.lastInspection().isValid() ? 
               pipeline.lastInspection().toString("yyyy-MM-dd") : "未设置");
    addInfoRow(maintenanceCard, "巡检周期", QString("%1 天").arg(pipeline.inspectionCycle()));
    addInfoRow(maintenanceCard, "养护单位", pipeline.maintenanceUnit());
    contentLayout->addWidget(maintenanceCard);
    
    // 备注卡片
    if (!pipeline.remarks().isEmpty()) {
        QWidget *remarksCard = createInfoCard("备注", "📝");
        QLabel *remarksLabel = new QLabel(pipeline.remarks(), remarksCard);
        remarksLabel->setWordWrap(true);
        remarksLabel->setStyleSheet(
            "QLabel {"
            "  color: #666;"
            "  font-size: 10px;"
            "  line-height: 1.6;"
            "  padding: 6px 0;"
            "}"
        );
        QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(remarksCard->layout());
        if (cardLayout) {
            cardLayout->addWidget(remarksLabel);
        }
        contentLayout->addWidget(remarksCard);
    }
    
    contentLayout->addStretch();
}

void EntityViewDialog::setFacility(const Facility &facility)
{
    m_isPipeline = false;
    clearContent();
    
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (!contentLayout) return;
    
    // 基本信息卡片
    QWidget *basicCard = createInfoCard("基本信息", "📋");
    addInfoRow(basicCard, "编号", facility.facilityId(), true);
    addInfoRow(basicCard, "名称", facility.facilityName());
    addInfoRow(basicCard, "类型", facility.facilityType());
    addInfoRow(basicCard, "数据库ID", QString::number(facility.id()));
    contentLayout->addWidget(basicCard);
    
    // 物理属性卡片
    QWidget *physicalCard = createInfoCard("物理属性", "🔧");
    addInfoRow(physicalCard, "规格型号", facility.spec());
    addInfoRow(physicalCard, "材质", facility.material());
    addInfoRow(physicalCard, "尺寸", facility.size());
    addInfoRow(physicalCard, "高程", QString("%1 m").arg(facility.elevationM(), 0, 'f', 2));
    contentLayout->addWidget(physicalCard);
    
    // 关联信息卡片
    QWidget *relationCard = createInfoCard("关联信息", "🔗");
    addInfoRow(relationCard, "关联管线", facility.pipelineId().isEmpty() ? "无" : facility.pipelineId());
    contentLayout->addWidget(relationCard);
    
    // 建设信息卡片
    QWidget *buildCard = createInfoCard("建设信息", "🏗️");
    addInfoRow(buildCard, "建设日期", facility.buildDate().isValid() ? 
               facility.buildDate().toString("yyyy-MM-dd") : "未设置");
    addInfoRow(buildCard, "施工单位", facility.builder());
    addInfoRow(buildCard, "产权单位", facility.owner());
    addInfoRow(buildCard, "资产价值", QString("%1 元").arg(facility.assetValue(), 0, 'f', 2));
    contentLayout->addWidget(buildCard);
    
    // 运维信息卡片
    QWidget *maintenanceCard = createInfoCard("运维信息", "⚙️");
    addInfoRow(maintenanceCard, "运行状态", facility.status());
    addInfoRow(maintenanceCard, "健康度", QString("%1 分").arg(facility.healthScore()), 
               false, facility.healthScore() < 60);
    addInfoRow(maintenanceCard, "上次维护", facility.lastMaintenance().isValid() ? 
               facility.lastMaintenance().toString("yyyy-MM-dd") : "未设置");
    addInfoRow(maintenanceCard, "下次维护", facility.nextMaintenance().isValid() ? 
               facility.nextMaintenance().toString("yyyy-MM-dd") : "未设置");
    addInfoRow(maintenanceCard, "养护单位", facility.maintenanceUnit());
    contentLayout->addWidget(maintenanceCard);
    
    // 二维码信息
    if (!facility.qrcodeUrl().isEmpty()) {
        QWidget *qrcodeCard = createInfoCard("二维码", "📱");
        addInfoRow(qrcodeCard, "二维码链接", facility.qrcodeUrl());
        contentLayout->addWidget(qrcodeCard);
    }
    
    // 备注卡片
    if (!facility.remarks().isEmpty()) {
        QWidget *remarksCard = createInfoCard("备注", "📝");
        QLabel *remarksLabel = new QLabel(facility.remarks(), remarksCard);
        remarksLabel->setWordWrap(true);
        remarksLabel->setStyleSheet(
            "QLabel {"
            "  color: #666;"
            "  font-size: 10px;"
            "  line-height: 1.6;"
            "  padding: 6px 0;"
            "}"
        );
        QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(remarksCard->layout());
        if (cardLayout) {
            cardLayout->addWidget(remarksLabel);
        }
        contentLayout->addWidget(remarksCard);
    }
    
    contentLayout->addStretch();
}

QWidget* EntityViewDialog::createInfoCard(const QString &title, const QString &icon)
{
    QWidget *card = new QWidget(m_contentWidget);
    card->setStyleSheet(
        "QWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "}"
    );
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);
    cardLayout->setContentsMargins(12, 10, 12, 10);
    
    // 标题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);
    
    QLabel *iconLabel = new QLabel(icon.isEmpty() ? "📌" : icon, card);
    QFont iconFont;
    iconFont.setPointSize(12);
    iconLabel->setFont(iconFont);
    titleLayout->addWidget(iconLabel);
    
    QLabel *titleLabel = new QLabel(title, card);
    QFont titleFont;
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #333;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    cardLayout->addLayout(titleLayout);
    
    // 分隔线
    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #ddd; max-height: 1px;");
    cardLayout->addWidget(line);
    
    return card;
}

void EntityViewDialog::addInfoRow(QWidget *card, const QString &label, const QString &value, bool highlight, bool warning)
{
    QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(card->layout());
    if (!cardLayout) return;
    
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);
    
    // 标签
    QLabel *labelWidget = new QLabel(label + ":", card);
    QFont labelFont;
    labelFont.setPointSize(10);
    labelFont.setBold(true);
    labelWidget->setFont(labelFont);
    labelWidget->setStyleSheet("color: #666; min-width: 80px;");
    labelWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    rowLayout->addWidget(labelWidget);
    
    // 值
    QLabel *valueWidget = new QLabel(value.isEmpty() ? "未设置" : value, card);
    QFont valueFont;
    valueFont.setPointSize(10);
    valueWidget->setFont(valueFont);
    if (warning) {
        valueWidget->setStyleSheet("color: #FF9800; font-weight: 600;");
    } else if (highlight) {
        valueWidget->setStyleSheet("color: #2196F3; font-weight: 600;");
    } else {
        valueWidget->setStyleSheet("color: #333;");
    }
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    rowLayout->addWidget(valueWidget, 1);
    
    cardLayout->addLayout(rowLayout);
}

void EntityViewDialog::clearContent()
{
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (!contentLayout) return;
    
    // 清除所有子控件（除了最后的stretch）
    QLayoutItem *item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void EntityViewDialog::onCloseClicked()
{
    accept();
}

void EntityViewDialog::onEditClicked()
{
    emit editRequested();
    accept();
}

