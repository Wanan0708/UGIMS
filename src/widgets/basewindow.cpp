// basewindow.cpp
#include "basewindow.h"
#include <QDebug>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QShowEvent>

const int BaseWindow::BORDER_WIDTH = 6;

BaseWindow::BaseWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(800, 600);  // 增加最小尺寸，避免拖动时自动最大化
    setMouseTracking(true);

    titleBar = new CustomTitleBar(this);
    contentContainer = new QWidget(this);
    contentContainer->setObjectName("contentContainer");

    centralWidgetContainer = new QWidget(this);
    centralWidgetContainer->setObjectName("centralWidgetContainer");

    auto mainLayout = new QVBoxLayout(centralWidgetContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(contentContainer);

    setCentralWidget(centralWidgetContainer);

    centralWidgetContainer->setMouseTracking(true);
    titleBar->setMouseTracking(true);
    contentContainer->setMouseTracking(true);

    // 设置标题栏标题和图标
    titleBar->setTitle("UGMIS");
    titleBar->setIcon(qApp->windowIcon());

    // 安装事件过滤器，确保子控件不吞掉鼠标移动，用于更新边框光标
    this->installEventFilter(this);
    centralWidgetContainer->installEventFilter(this);
    contentContainer->installEventFilter(this);
    // 全局监听鼠标移动，保证在复杂子层级上也能更新光标
    qApp->installEventFilter(this);
}

void BaseWindow::setContentWidget(QWidget *content) {
    auto layout = contentContainer->layout();
    if (layout) {
        delete layout;
    }
    layout = new QVBoxLayout(contentContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(content);
}

bool BaseWindow::event(QEvent *event) {
    // 监听窗口状态变化，同步标题栏按钮
    if (event->type() == QEvent::WindowStateChange) {
        if (titleBar) {
            titleBar->setMaximized(isMaximized());
        }
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // 只有在非拖拽、非左键按下、非最大化时更新光标样式
        if (!m_resizing && !(mouseEvent->buttons() & Qt::LeftButton) && !isMaximized()) {
            QPoint pos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
            ResizeDirection dir = getResizeDirection(pos);

            switch (dir) {
            case Top:
            case Bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case Left:
            case Right:
                setCursor(Qt::SizeHorCursor);
                break;
            case TopLeft:
            case BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case TopRight:
            case BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            default:
                setCursor(Qt::ArrowCursor);
                break;
            }
        }
    } else if (event->type() == QEvent::Leave) {
        // 离开窗口时恢复箭头光标
        unsetCursor();
    }
    return QMainWindow::event(event);
}

bool BaseWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // 窗口最大化时不显示缩放光标
        if (!m_resizing && !(mouseEvent->buttons() & Qt::LeftButton) && !isMaximized()) {
            // 仅当鼠标位于本窗口上时处理
            const QPoint globalPos = mouseEvent->globalPosition().toPoint();
            QWidget *w = QApplication::widgetAt(globalPos);
            if (w && w->window() == this) {
                // 统一使用全局坐标映射到窗口坐标，避免子控件局部坐标误差
                QPoint pos = mapFromGlobal(globalPos);
                ResizeDirection dir = getResizeDirection(pos);

                switch (dir) {
                case Top:
                case Bottom:
                    setCursor(Qt::SizeVerCursor);
                    break;
                case Left:
                case Right:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case TopLeft:
                case BottomRight:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case TopRight:
                case BottomLeft:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                default:
                    setCursor(Qt::ArrowCursor);
                    break;
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

BaseWindow::ResizeDirection BaseWindow::getResizeDirection(const QPoint &pos) const {
    const int bw = BORDER_WIDTH;
    const QRect r = rect();
    int dir = None;

    // 使用 if（不是 else if），允许角落同时触发两个方向
    if (pos.y() <= bw) dir |= Top;
    if (pos.y() >= r.height() - bw) dir |= Bottom;
    if (pos.x() <= bw) dir |= Left;
    if (pos.x() >= r.width() - bw) dir |= Right;

    return static_cast<ResizeDirection>(dir); // 显式转回 enum
}

void BaseWindow::mousePressEvent(QMouseEvent *event) {
    // 窗口最大化时，禁止边框缩放
    if (event->button() == Qt::LeftButton && !isMaximized() && !titleBar->geometry().contains(event->pos())) {
        m_resizeDir = getResizeDirection(event->pos());
        if (m_resizeDir != None) {
            m_resizing = true;
            m_startGeometry = geometry();
            m_startMousePos = event->globalPos();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void BaseWindow::mouseMoveEvent(QMouseEvent *event) {
    // 如果正在拖拽缩放，处理缩放逻辑
    if (m_resizing) {
        const QPoint delta = event->globalPosition().toPoint() - m_startMousePos;
        QRect newGeom = m_startGeometry;

        int minWidth = this->minimumWidth();
        int minHeight = this->minimumHeight();
        
        // 获取屏幕可用区域，避免窗口超出屏幕导致自动最大化
        auto screen = QGuiApplication::screenAt(event->globalPosition().toPoint());
        if (!screen) screen = QGuiApplication::primaryScreen();
        QRect screenGeo = screen->availableGeometry();
        int maxWidth = screenGeo.width() - 20;  // 留出一些边距
        int maxHeight = screenGeo.height() - 20;

        // 处理垂直方向
        if (m_resizeDir & Top) {
            int newTop = m_startGeometry.top() + delta.y();
            int newHeight = m_startGeometry.bottom() - newTop + 1;
            if (newHeight >= minHeight && newHeight <= maxHeight) {
                newGeom.setTop(newTop);
            } else if (newHeight < minHeight) {
                newGeom.setTop(m_startGeometry.bottom() - minHeight + 1);
            } else {
                newGeom.setTop(m_startGeometry.bottom() - maxHeight + 1);
            }
        } else if (m_resizeDir & Bottom) {
            int newBottom = m_startGeometry.bottom() + delta.y();
            int newHeight = newBottom - m_startGeometry.top() + 1;
            if (newHeight >= minHeight && newHeight <= maxHeight) {
                newGeom.setBottom(newBottom);
            } else if (newHeight < minHeight) {
                newGeom.setBottom(m_startGeometry.top() + minHeight - 1);
            } else {
                newGeom.setBottom(m_startGeometry.top() + maxHeight - 1);
            }
        }

        // 处理水平方向
        if (m_resizeDir & Left) {
            int newLeft = m_startGeometry.left() + delta.x();
            int newWidth = m_startGeometry.right() - newLeft + 1;
            if (newWidth >= minWidth && newWidth <= maxWidth) {
                newGeom.setLeft(newLeft);
            } else if (newWidth < minWidth) {
                newGeom.setLeft(m_startGeometry.right() - minWidth + 1);
            } else {
                newGeom.setLeft(m_startGeometry.right() - maxWidth + 1);
            }
        } else if (m_resizeDir & Right) {
            int newRight = m_startGeometry.right() + delta.x();
            int newWidth = newRight - m_startGeometry.left() + 1;
            if (newWidth >= minWidth && newWidth <= maxWidth) {
                newGeom.setRight(newRight);
            } else if (newWidth < minWidth) {
                newGeom.setRight(m_startGeometry.left() + minWidth - 1);
            } else {
                newGeom.setRight(m_startGeometry.left() + maxWidth - 1);
            }
        }

        setGeometry(newGeom);
        event->accept();
        return;
    }

    QMainWindow::mouseMoveEvent(event);
}

void BaseWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_resizing = false;
        m_resizeDir = None;
        unsetCursor();
    }
    QMainWindow::mouseReleaseEvent(event);
}

void BaseWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    // 同步标题栏的最大化状态与窗口当前状态
    if (titleBar) {
        titleBar->setMaximized(isMaximized());
    }
}

