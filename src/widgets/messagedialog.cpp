#include "messagedialog.h"
#include "core/auth/sessionmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QFont>

MessageDialog::MessageDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentFilter(FILTER_ALL)
    , m_unreadCount(0)
{
    setWindowTitle("消息中心");
    setMinimumSize(600, 500);
    setModal(false);
    
    setupUI();
    loadMessages();
}

MessageDialog::~MessageDialog()
{
}

void MessageDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 筛选按钮组
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(5);
    
    m_filterGroup = new QButtonGroup(this);
    
    m_allButton = new QPushButton("全部", this);
    m_unreadButton = new QPushButton("未读", this);
    m_systemButton = new QPushButton("系统", this);
    m_workorderButton = new QPushButton("工单", this);
    m_warningButton = new QPushButton("警告", this);
    
    m_filterGroup->addButton(m_allButton, FILTER_ALL);
    m_filterGroup->addButton(m_unreadButton, FILTER_UNREAD);
    m_filterGroup->addButton(m_systemButton, FILTER_SYSTEM);
    m_filterGroup->addButton(m_workorderButton, FILTER_WORKORDER);
    m_filterGroup->addButton(m_warningButton, FILTER_WARNING);
    
    // 设置按钮样式
    for (auto btn : {m_allButton, m_unreadButton, m_systemButton, m_workorderButton, m_warningButton}) {
        btn->setCheckable(true);
        btn->setFixedHeight(30);
        filterLayout->addWidget(btn);
    }
    
    m_allButton->setChecked(true);
    filterLayout->addStretch();
    
    mainLayout->addLayout(filterLayout);
    
    // 消息列表
    m_messageList = new QListWidget(this);
    m_messageList->setAlternatingRowColors(true);
    mainLayout->addWidget(m_messageList);
    
    // 状态标签
    m_statusLabel = new QLabel("共 0 条消息", this);
    mainLayout->addWidget(m_statusLabel);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_markAllReadBtn = new QPushButton("标记全部已读", this));
    buttonLayout->addWidget(m_deleteReadBtn = new QPushButton("删除已读", this));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshBtn = new QPushButton("刷新", this));
    buttonLayout->addWidget(m_closeBtn = new QPushButton("关闭", this));
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_filterGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MessageDialog::onFilterChanged);
    connect(m_messageList, &QListWidget::itemClicked, this, &MessageDialog::onMessageItemClicked);
    connect(m_messageList, &QListWidget::itemDoubleClicked, this, &MessageDialog::onMessageItemDoubleClicked);
    connect(m_markAllReadBtn, &QPushButton::clicked, this, &MessageDialog::onMarkAllReadClicked);
    connect(m_deleteReadBtn, &QPushButton::clicked, this, &MessageDialog::onDeleteReadClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MessageDialog::onRefreshClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &MessageDialog::onCloseClicked);
}

void MessageDialog::loadMessages()
{
    // TODO: 从数据库加载消息
    // 目前使用模拟数据
    m_allMessages.clear();
    
    // 模拟消息数据
    QVariantMap msg1;
    msg1["id"] = 1;
    msg1["type"] = "system";
    msg1["title"] = "系统通知";
    msg1["content"] = "系统将在今晚22:00进行维护，预计持续1小时";
    msg1["is_read"] = false;
    msg1["created_at"] = QDateTime::currentDateTime().addDays(-1);
    m_allMessages.append(msg1);
    
    QVariantMap msg2;
    msg2["id"] = 2;
    msg2["type"] = "workorder";
    msg2["title"] = "工单通知";
    msg2["content"] = "您有新的工单需要处理：WO-2025-001";
    msg2["is_read"] = false;
    msg2["created_at"] = QDateTime::currentDateTime().addSecs(-2 * 3600);  // 2小时前
    m_allMessages.append(msg2);
    
    QVariantMap msg3;
    msg3["id"] = 3;
    msg3["type"] = "warning";
    msg3["title"] = "警告";
    msg3["content"] = "管线 P001 健康度低于阈值，需要及时处理";
    msg3["is_read"] = true;
    msg3["created_at"] = QDateTime::currentDateTime().addDays(-3);
    m_allMessages.append(msg3);
    
    updateUnreadCount();
    filterMessages(m_currentFilter);
}

void MessageDialog::filterMessages(int filterType)
{
    m_currentFilter = filterType;
    m_messageList->clear();
    
    for (const QVariantMap &msg : m_allMessages) {
        bool shouldShow = false;
        
        switch (filterType) {
        case FILTER_ALL:
            shouldShow = true;
            break;
        case FILTER_UNREAD:
            shouldShow = !msg["is_read"].toBool();
            break;
        case FILTER_SYSTEM:
            shouldShow = msg["type"].toString() == "system";
            break;
        case FILTER_WORKORDER:
            shouldShow = msg["type"].toString() == "workorder";
            break;
        case FILTER_WARNING:
            shouldShow = msg["type"].toString() == "warning";
            break;
        }
        
        if (shouldShow) {
            QListWidgetItem *item = new QListWidgetItem();
            updateMessageItem(item, msg);
            m_messageList->addItem(item);
        }
    }
    
    // 更新状态标签
    int totalCount = m_messageList->count();
    int unreadCount = 0;
    for (int i = 0; i < m_messageList->count(); ++i) {
        QListWidgetItem *item = m_messageList->item(i);
        QVariantMap msg = item->data(Qt::UserRole).toMap();
        if (!msg["is_read"].toBool()) {
            unreadCount++;
        }
    }
    m_statusLabel->setText(QString("共 %1 条消息，%2 条未读").arg(totalCount).arg(unreadCount));
}

void MessageDialog::updateMessageItem(QListWidgetItem *item, const QVariantMap &message)
{
    QString type = message["type"].toString();
    QString title = message["title"].toString();
    QString content = message["content"].toString();
    bool isRead = message["is_read"].toBool();
    QDateTime createdAt = message["created_at"].toDateTime();
    
    // 设置图标
    item->setIcon(getMessageTypeIcon(type));
    
    // 设置文本
    QString displayText = QString("[%1] %2\n%3\n%4")
                          .arg(getMessageTypeName(type))
                          .arg(title)
                          .arg(content)
                          .arg(createdAt.toString("yyyy-MM-dd hh:mm"));
    
    item->setText(displayText);
    
    // 未读消息使用粗体
    QFont font = item->font();
    font.setBold(!isRead);
    item->setFont(font);
    
    // 存储消息数据
    item->setData(Qt::UserRole, message);
}

QIcon MessageDialog::getMessageTypeIcon(const QString &type) const
{
    // 使用 Qt 标准图标
    if (type == "system") {
        return QIcon(":/new/prefix1/images/Message.png");
    } else if (type == "workorder") {
        return QIcon(":/new/prefix1/images/Message.png");
    } else if (type == "warning") {
        return QIcon(":/new/prefix1/images/Message.png");
    }
    return QIcon(":/new/prefix1/images/NoMessage.png");
}

QString MessageDialog::getMessageTypeName(const QString &type) const
{
    if (type == "system") return "系统";
    if (type == "workorder") return "工单";
    if (type == "warning") return "警告";
    if (type == "info") return "信息";
    return "未知";
}

void MessageDialog::onFilterChanged(int filterType)
{
    filterMessages(filterType);
}

void MessageDialog::onMessageItemClicked(QListWidgetItem *item)
{
    QVariantMap message = item->data(Qt::UserRole).toMap();
    int messageId = message["id"].toInt();
    
    // 标记为已读
    if (!message["is_read"].toBool()) {
        markMessageAsRead(messageId);
        message["is_read"] = true;
        item->setData(Qt::UserRole, message);
        updateMessageItem(item, message);
        updateUnreadCount();
    }
}

void MessageDialog::onMessageItemDoubleClicked(QListWidgetItem *item)
{
    QVariantMap message = item->data(Qt::UserRole).toMap();
    QString title = message["title"].toString();
    QString content = message["content"].toString();
    
    QMessageBox::information(this, title, content);
}

void MessageDialog::onMarkAllReadClicked()
{
    int count = 0;
    for (int i = 0; i < m_messageList->count(); ++i) {
        QListWidgetItem *item = m_messageList->item(i);
        QVariantMap message = item->data(Qt::UserRole).toMap();
        if (!message["is_read"].toBool()) {
            int messageId = message["id"].toInt();
            markMessageAsRead(messageId);
            message["is_read"] = true;
            item->setData(Qt::UserRole, message);
            updateMessageItem(item, message);
            count++;
        }
    }
    
    if (count > 0) {
        updateUnreadCount();
        QMessageBox::information(this, "提示", QString("已标记 %1 条消息为已读").arg(count));
    }
}

void MessageDialog::onDeleteReadClicked()
{
    int count = 0;
    QList<QListWidgetItem*> itemsToDelete;
    
    for (int i = 0; i < m_messageList->count(); ++i) {
        QListWidgetItem *item = m_messageList->item(i);
        QVariantMap message = item->data(Qt::UserRole).toMap();
        if (message["is_read"].toBool()) {
            int messageId = message["id"].toInt();
            deleteMessage(messageId);
            itemsToDelete.append(item);
            count++;
        }
    }
    
    for (QListWidgetItem *item : itemsToDelete) {
        delete m_messageList->takeItem(m_messageList->row(item));
    }
    
    if (count > 0) {
        updateUnreadCount();
        filterMessages(m_currentFilter);
        QMessageBox::information(this, "提示", QString("已删除 %1 条已读消息").arg(count));
    }
}

void MessageDialog::onRefreshClicked()
{
    loadMessages();
    QMessageBox::information(this, "提示", "消息已刷新");
}

void MessageDialog::onCloseClicked()
{
    close();
}

void MessageDialog::markMessageAsRead(int messageId)
{
    // TODO: 更新数据库
    qDebug() << "Mark message as read:" << messageId;
}

void MessageDialog::deleteMessage(int messageId)
{
    // TODO: 从数据库删除
    qDebug() << "Delete message:" << messageId;
}

void MessageDialog::updateUnreadCount()
{
    m_unreadCount = 0;
    for (const QVariantMap &msg : m_allMessages) {
        if (!msg["is_read"].toBool()) {
            m_unreadCount++;
        }
    }
    
    emit unreadCountChanged(m_unreadCount);
}

int MessageDialog::getUnreadCount() const
{
    return m_unreadCount;
}

void MessageDialog::refreshMessages()
{
    loadMessages();
}

