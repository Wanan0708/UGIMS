#ifndef COLLAPSIBLEGROUPBOX_H
#define COLLAPSIBLEGROUPBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEvent>

/**
 * @brief 可折叠的分组容器，支持展开/折叠动画
 * 
 * 特点：
 * - 可点击的标题栏，带折叠箭头指示器
 * - 平滑的展开/折叠动画效果
 * - 支持自定义内容布局
 * - 现代化的 UI 设计
 */
class CollapsibleGroupBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

public:
    explicit CollapsibleGroupBox(const QString &title, QWidget *parent = nullptr);
    ~CollapsibleGroupBox();

    /**
     * @brief 设置内容布局
     * @param layout 要添加到内容区域的布局
     */
    void setContentLayout(QLayout *layout);
    
    /**
     * @brief 设置展开/折叠状态
     * @param expanded true=展开, false=折叠
     * @param animated 是否使用动画
     */
    void setExpanded(bool expanded, bool animated = true);
    
    /**
     * @brief 获取当前展开状态
     * @return true=已展开, false=已折叠
     */
    bool isExpanded() const { return m_expanded; }
    
    /**
     * @brief 设置标题文本
     */
    void setTitle(const QString &title);
    
    /**
     * @brief 获取标题文本
     */
    QString title() const;

signals:
    /**
     * @brief 展开状态改变信号
     * @param expanded 新的展开状态
     */
    void expandedChanged(bool expanded);

private slots:
    void onHeaderClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void updateArrowIcon();
    
    // UI 组件
    QVBoxLayout *m_mainLayout;        // 主布局
    QWidget *m_headerWidget;          // 标题栏容器
    QHBoxLayout *m_headerLayout;      // 标题栏布局
    QLabel *m_arrowLabel;             // 折叠箭头图标
    QLabel *m_titleLabel;             // 标题文本
    QWidget *m_contentWidget;         // 内容容器
    QVBoxLayout *m_contentLayout;     // 内容布局
    
    // 状态
    QString m_title;                  // 标题文本
    bool m_expanded;                  // 展开状态
    
    // 动画
    QPropertyAnimation *m_animation;  // 高度动画
    
    // 常量
    static const int ANIMATION_DURATION = 200;  // 动画持续时间（毫秒）
};

#endif // COLLAPSIBLEGROUPBOX_H
