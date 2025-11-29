#include "widgets/collapsiblegroupbox.h"
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QDebug>

CollapsibleGroupBox::CollapsibleGroupBox(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
    , m_expanded(true)
    , m_animation(nullptr)
{
    setupUI();
}

CollapsibleGroupBox::~CollapsibleGroupBox()
{
}

void CollapsibleGroupBox::setupUI()
{
    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // ========== 标题栏 ==========
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("collapsibleHeader");
    m_headerWidget->setCursor(Qt::PointingHandCursor);
    m_headerWidget->setStyleSheet(
        "#collapsibleHeader {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 #f8f8f8, stop:1 #ececec);"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "}"
        "#collapsibleHeader:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 #e8f4ff, stop:1 #d6ecff);"
        "  border-color: #40a9ff;"
        "}"
    );
    
    // 标题栏布局
    m_headerLayout = new QHBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(4, 4, 4, 4);
    m_headerLayout->setSpacing(8);
    
    // 箭头图标
    m_arrowLabel = new QLabel(this);
    m_arrowLabel->setFixedSize(16, 16);
    m_arrowLabel->setAlignment(Qt::AlignCenter);
    m_arrowLabel->setStyleSheet(
        "font-size: 12pt;"
        "font-weight: bold;"
        "color: #666666;"
    );
    
    // 标题文本
    m_titleLabel = new QLabel(m_title, this);
    m_titleLabel->setStyleSheet(
        "font-weight: bold;"
        "font-size: 10pt;"
        "color: #333333;"
    );
    
    m_headerLayout->addWidget(m_arrowLabel);
    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    
    // 标题栏点击事件
    m_headerWidget->installEventFilter(this);
    
    // ========== 内容区域 ==========
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("collapsibleContent");
    m_contentWidget->setStyleSheet(
        "#collapsibleContent {"
        "  background: white;"
        "  border: 1px solid #e0e0e0;"
        "  border-top: none;"
        "  border-radius: 0 0 4px 4px;"
        "  padding: 4px;"
        "}"
    );
    
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(4, 4, 4, 4);
    m_contentLayout->setSpacing(4);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_headerWidget);
    m_mainLayout->addWidget(m_contentWidget);
    
    // 创建动画
    m_animation = new QPropertyAnimation(m_contentWidget, "maximumHeight", this);
    m_animation->setDuration(ANIMATION_DURATION);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // 更新箭头图标
    updateArrowIcon();
}

bool CollapsibleGroupBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_headerWidget && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onHeaderClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CollapsibleGroupBox::setContentLayout(QLayout *layout)
{
    if (!layout) {
        return;
    }
    
    // 清空现有布局内容
    QLayoutItem *item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        delete item;
    }
    
    // 添加新布局的所有子项
    while (layout->count() > 0) {
        QLayoutItem *childItem = layout->takeAt(0);
        if (childItem->widget()) {
            m_contentLayout->addWidget(childItem->widget());
        } else if (childItem->layout()) {
            m_contentLayout->addLayout(childItem->layout());
        } else if (childItem->spacerItem()) {
            m_contentLayout->addItem(childItem->spacerItem());
        }
        delete childItem;
    }
    
    delete layout;
}

void CollapsibleGroupBox::setExpanded(bool expanded, bool animated)
{
    if (m_expanded == expanded) {
        return;
    }
    
    m_expanded = expanded;
    
    if (animated) {
        // 使用动画
        int startHeight = m_expanded ? 0 : m_contentWidget->sizeHint().height();
        int endHeight = m_expanded ? m_contentWidget->sizeHint().height() : 0;
        
        m_animation->setStartValue(startHeight);
        m_animation->setEndValue(endHeight);
        m_animation->start();
        
        // 动画结束后设置最终状态
        connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
            if (!m_expanded) {
                m_contentWidget->setMaximumHeight(0);
            } else {
                m_contentWidget->setMaximumHeight(16777215); // QWIDGETSIZE_MAX
            }
        }, Qt::UniqueConnection);
    } else {
        // 直接设置
        if (m_expanded) {
            m_contentWidget->setMaximumHeight(16777215);
            m_contentWidget->setVisible(true);
        } else {
            m_contentWidget->setMaximumHeight(0);
            m_contentWidget->setVisible(false);
        }
    }
    
    updateArrowIcon();
    emit expandedChanged(m_expanded);
}

void CollapsibleGroupBox::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(title);
}

QString CollapsibleGroupBox::title() const
{
    return m_title;
}

void CollapsibleGroupBox::onHeaderClicked()
{
    setExpanded(!m_expanded, true);
}

void CollapsibleGroupBox::updateArrowIcon()
{
    // 展开状态：▼  折叠状态：▶
    m_arrowLabel->setText(m_expanded ? "▼" : "▶");
}
