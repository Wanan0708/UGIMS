// customtitlebar.cpp
#include "customtitlebar.h"
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QPalette>
#include <QStyleOption>
#include <QPainter>

CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(32);

    iconLabel = new QLabel(this);
    iconLabel->setFixedSize(28, 28);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setScaledContents(false);

    // 中间标题文字：程序名 UGMIS
    titleLabel = new QLabel("UGMIS");
    titleLabel->setObjectName("titleLabel");

    minButton = new QPushButton();
    maxButton = new QPushButton();
    closeButton = new QPushButton();
    
    // 设置按钮的对象名称
    minButton->setObjectName("minButton");
    maxButton->setObjectName("maxButton");
    closeButton->setObjectName("closeButton");

    // 设置图标（从资源加载）
    minButton->setIcon(QIcon(":/new/prefix1/images/minimize.png"));
    // 默认窗口是正常状态，此时按钮图标应为“最大化”
    maxButton->setIcon(QIcon(":/new/prefix1/images/maximize.png"));
    closeButton->setIcon(QIcon(":/new/prefix1/images/close.png"));

    // 设置图标大小（必须！否则可能不显示）
    minButton->setIconSize(QSize(16, 16));
    maxButton->setIconSize(QSize(16, 16));
    closeButton->setIconSize(QSize(16, 16));

    for (auto btn : {minButton, maxButton, closeButton}) {
        btn->setFixedSize(40, 32);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setText("");
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 0);
    layout->setSpacing(0);

    // 布局：左侧图标，中间居中标题，右侧按钮
    layout->addWidget(iconLabel);
    layout->addSpacing(6);
    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(minButton);
    layout->addWidget(maxButton);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &CustomTitleBar::onClose);
    connect(minButton, &QPushButton::clicked, this, &CustomTitleBar::onMinimize);
    connect(maxButton, &QPushButton::clicked, this, &CustomTitleBar::onMaximizeRestore);
}

// 重写paintEvent来确保样式表能正确应用
void CustomTitleBar::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event) {
    qDebug() << "CustomTitleBar::mousePressEvent - button:" << event->button() 
             << "isMaximized:" << m_isMaximized
             << "closeButton underMouse:" << closeButton->underMouse()
             << "maxButton underMouse:" << maxButton->underMouse()
             << "minButton underMouse:" << minButton->underMouse();
    
    if (event->button() == Qt::LeftButton && !m_isMaximized) {
        if (!closeButton->underMouse() && !maxButton->underMouse() && !minButton->underMouse()) {
            m_dragPos = event->globalPos() - window()->frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !m_isMaximized) {
        if (!closeButton->underMouse() && !maxButton->underMouse() && !minButton->underMouse()) {
            auto targetPos = event->globalPos() - m_dragPos;
            auto screen = QGuiApplication::screenAt(event->globalPos());
            if (!screen) screen = QGuiApplication::primaryScreen();
            QRect screenGeo = screen->availableGeometry();

            // 最小可见区域：允许窗口部分出屏，但保留 kMinVisibleMargin 在屏内
            int minVisible = kMinVisibleMargin;
            int minX = screenGeo.left() - width() + minVisible;
            int maxX = screenGeo.right() - minVisible;
            int minY = screenGeo.top(); // 标题栏一般要求在屏内，避免拖不回
            int maxY = screenGeo.bottom() - 1; // 底部可部分出屏

            targetPos.setX(qBound(minX, targetPos.x(), maxX));
            targetPos.setY(qBound(minY, targetPos.y(), maxY));

            window()->move(targetPos);
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        onMaximizeRestore();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CustomTitleBar::onClose() {
    qDebug() << "CustomTitleBar::onClose() called";
    window()->close();
}

void CustomTitleBar::onMinimize() {
    qDebug() << "CustomTitleBar::onMinimize() called";
    window()->showMinimized();
}

void CustomTitleBar::onMaximizeRestore() {
    qDebug() << "CustomTitleBar::onMaximizeRestore() called, current state:" << m_isMaximized;
    if (m_isMaximized) {
        window()->showNormal();
        setMaximized(false);
    } else {
        window()->showMaximized();
        setMaximized(true);
    }
}

void CustomTitleBar::setTitle(const QString &title)
{
    if (titleLabel) {
        titleLabel->setText(title);
    }
}

void CustomTitleBar::setIcon(const QIcon &icon)
{
    if (iconLabel) {
        // 使用QIcon的pixmap方法获取合适尺寸的图标，确保不被拉伸
        QPixmap pixmap = icon.pixmap(QSize(20, 20), QIcon::Normal, QIcon::Off);
        // 如果图标不存在，使用缩放的原始图标
        if (pixmap.isNull()) {
            pixmap = icon.pixmap(20, 20);
        }
        // 缩放至标签大小，保持长宽比
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaledToHeight(20, Qt::SmoothTransformation);
        }
        iconLabel->setPixmap(pixmap);
    }
}

void CustomTitleBar::setMaximized(bool maximized)
{
    m_isMaximized = maximized;
    // 当窗口最大化时，按钮显示“还原”图标；正常时显示“最大化”图标
    if (m_isMaximized) {
        maxButton->setIcon(QIcon(":/new/prefix1/images/restore.png"));
    } else {
        maxButton->setIcon(QIcon(":/new/prefix1/images/maximize.png"));
    }
}
