#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class QVBoxLayout;
class QHBoxLayout;
class QButtonGroup;

/**
 * @brief 消息对话框
 * 显示系统消息和通知列表
 */
class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageDialog(QWidget *parent = nullptr);
    ~MessageDialog();

    // 刷新消息列表
    void refreshMessages();
    
    // 获取未读消息数量
    int getUnreadCount() const;

signals:
    // 未读消息数量变化
    void unreadCountChanged(int count);

private slots:
    void onFilterChanged(int filterType);
    void onMessageItemClicked(QListWidgetItem *item);
    void onMessageItemDoubleClicked(QListWidgetItem *item);
    void onMarkAllReadClicked();
    void onDeleteReadClicked();
    void onRefreshClicked();
    void onCloseClicked();

private:
    void setupUI();
    void loadMessages();
    void filterMessages(int filterType);
    void updateMessageItem(QListWidgetItem *item, const QVariantMap &message);
    void markMessageAsRead(int messageId);
    void deleteMessage(int messageId);
    void updateUnreadCount();
    
    // 获取消息类型图标
    QIcon getMessageTypeIcon(const QString &type) const;
    // 获取消息类型显示名称
    QString getMessageTypeName(const QString &type) const;

private:
    // UI 组件
    QListWidget *m_messageList;
    QPushButton *m_allButton;
    QPushButton *m_unreadButton;
    QPushButton *m_systemButton;
    QPushButton *m_workorderButton;
    QPushButton *m_warningButton;
    QButtonGroup *m_filterGroup;
    
    QPushButton *m_markAllReadBtn;
    QPushButton *m_deleteReadBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_closeBtn;
    
    QLabel *m_statusLabel;
    
    // 数据
    QList<QVariantMap> m_allMessages;
    int m_currentFilter;
    int m_unreadCount;
    
    // 消息类型常量
    static const int FILTER_ALL = 0;
    static const int FILTER_UNREAD = 1;
    static const int FILTER_SYSTEM = 2;
    static const int FILTER_WORKORDER = 3;
    static const int FILTER_WARNING = 4;
};

#endif // MESSAGEDIALOG_H

