#include "helpdialog.h"
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
#include <QSizePolicy>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_headerWidget(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_closeBtn(nullptr)
{
    setWindowTitle("帮助");
    setMinimumSize(550, 550);
    resize(600, 600);
    // 注意：不要使用 WA_DeleteOnClose，因为对话框是栈对象创建的
    
    setupUI();
}

HelpDialog::~HelpDialog()
{
}

void HelpDialog::setupUI()
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
    QLabel *iconLabel = new QLabel("💡", m_headerWidget);
    QFont iconFont;
    iconFont.setPointSize(20);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border: none; background: transparent;");
    headerLayout->addWidget(iconLabel);
    
    // 标题
    QLabel *titleLabel = new QLabel("使用帮助", m_headerWidget);
    QFont titleFont;
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: #333; border: none; background: transparent;");
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
    
    // 欢迎文字
    QLabel *welcomeLabel = new QLabel("欢迎使用城市地下管网智能管理系统", m_contentWidget);
    QFont welcomeFont;
    welcomeFont.setPointSize(12);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setStyleSheet("color: #333; padding: 8px 0; border: none; background: transparent;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(welcomeLabel);
    
    // 功能模块卡片（使用网格布局）
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(12);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    
    // 数据与地图
    QWidget *card1 = createFeatureCard("📊", "数据与地图", 
        "导入管网数据、下载离线地图\n管理地图图层和瓦片数据");
    gridLayout->addWidget(card1, 0, 0);
    
    // 空间分析
    QWidget *card2 = createFeatureCard("🔍", "空间分析", 
        "爆管影响分析、连通性分析\n管网健康度评估");
    gridLayout->addWidget(card2, 0, 1);
    
    // 工单与资产
    QWidget *card3 = createFeatureCard("📋", "工单与资产", 
        "工单管理、资产台账管理\n设备信息查询与编辑");
    gridLayout->addWidget(card3, 1, 0);
    
    // 工具
    QWidget *card4 = createFeatureCard("⚙️", "工具", 
        "系统设置、在线帮助\n距离测量、面积测量");
    gridLayout->addWidget(card4, 1, 1);
    
    contentLayout->addLayout(gridLayout);
    
    // 使用提示
    QFrame *tipsFrame = new QFrame(m_contentWidget);
    tipsFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #ffffff;"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "}"
    );
    QVBoxLayout *tipsLayout = new QVBoxLayout(tipsFrame);
    tipsLayout->setSpacing(8);
    tipsLayout->setContentsMargins(12, 12, 12, 12);
    
    QLabel *tipsTitle = new QLabel("💡 使用提示", tipsFrame);
    QFont tipsTitleFont;
    tipsTitleFont.setPointSize(11);
    tipsTitleFont.setBold(true);
    tipsTitle->setFont(tipsTitleFont);
    tipsTitle->setStyleSheet("color: #333; border: none; background: transparent;");
    tipsLayout->addWidget(tipsTitle);
    
    QLabel *tipsContent = new QLabel(
        "• 点击左侧设备树可快速定位到地图上的设备位置\n"
        "• 使用工具栏的缩放和平移工具可以更好地查看地图\n"
        "• 双击地图上的设备可以查看和编辑详细信息\n"
        "• 使用空间分析功能可以帮助您做出更好的决策",
        tipsFrame
    );
    tipsContent->setWordWrap(true);
    QFont tipsContentFont;
    tipsContentFont.setPointSize(10);
    tipsContent->setFont(tipsContentFont);
    tipsContent->setStyleSheet("color: #666; line-height: 1.6; border: none; background: transparent;");
    tipsLayout->addWidget(tipsContent);
    
    contentLayout->addWidget(tipsFrame);
    
    // 添加弹性空间
    contentLayout->addStretch();
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    // ========== 底部按钮区域 ==========
    QWidget *footerWidget = new QWidget(this);
    footerWidget->setStyleSheet("background-color: #f5f5f5; border-top: 1px solid #ddd;");
    footerWidget->setFixedHeight(50);
    
    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 8, 20, 8);
    footerLayout->addStretch();
    
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
    connect(m_closeBtn, &QPushButton::clicked, this, &HelpDialog::accept);
}

QWidget* HelpDialog::createFeatureCard(const QString &icon, const QString &title, const QString &description)
{
    QWidget *card = new QWidget(m_contentWidget);
    card->setStyleSheet(
        "QWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "}"
        "QLabel {"
        "  border: none;"
        "  background: transparent;"
        "}"
    );
    card->setMinimumHeight(140);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    
    // 图标
    QLabel *iconLabel = new QLabel(icon, card);
    QFont iconFont;
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border: none; background: transparent;");
    cardLayout->addWidget(iconLabel);
    
    // 标题
    QLabel *titleLabel = new QLabel(title, card);
    QFont titleFont;
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #333; border: none; background: transparent;");
    cardLayout->addWidget(titleLabel);
    
    // 描述
    QLabel *descLabel = new QLabel(description, card);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    QFont descFont;
    descFont.setPointSize(9);
    descLabel->setFont(descFont);
    descLabel->setStyleSheet("color: #666; line-height: 1.5; border: none; background: transparent;");
    cardLayout->addWidget(descLabel);
    
    cardLayout->addStretch();
    
    return card;
}

void HelpDialog::onCloseClicked()
{
    accept();
}

