// customtitlebar.h
#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

class CustomTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = nullptr);

    // 设置标题文本
    void setTitle(const QString &title);
    // 设置左侧图标
    void setIcon(const QIcon &icon);
    // 同步窗口最大化状态（用于启动时或外部状态变化时）
    void setMaximized(bool maximized);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;  // 添加这一行

private slots:
    void onClose();
    void onMinimize();
    void onMaximizeRestore();

private:
    QLabel *iconLabel;
    QLabel *titleLabel;
    QPushButton *minButton;
    QPushButton *maxButton;
    QPushButton *closeButton;

    bool m_isMaximized = false;
    QPoint m_dragPos;
    const int kMinVisibleMargin = 80; // 最小可见边距（像素）
};

#endif // CUSTOMTITLEBAR_H