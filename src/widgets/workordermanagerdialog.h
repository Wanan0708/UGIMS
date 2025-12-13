#ifndef WORKORDERMANAGERDIALOG_H
#define WORKORDERMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QLabel>
#include "core/models/workorder.h"
#include "core/workorder/workorderstatustransition.h"

class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;

/**
 * @brief 工单管理对话框
 * 提供工单的查看、创建、编辑、筛选等功能
 */
class WorkOrderManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WorkOrderManagerDialog(QWidget *parent = nullptr);
    ~WorkOrderManagerDialog();

private slots:
    void onRefreshClicked();
    void onCreateClicked();
    void onEditClicked();
    void onViewClicked();
    void onDeleteClicked();
    void onFilterChanged();
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);
    
    // 状态转换槽函数
    void onAssignClicked();
    void onStartClicked();
    void onCompleteClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupFilterPanel();
    void setupTable();
    void loadWorkOrders();
    void refreshTable();
    WorkOrder getSelectedWorkOrder();
    QString getStatusDisplayName(const QString &status);
    QString getTypeDisplayName(const QString &type);
    QString getPriorityDisplayName(const QString &priority);

private:
    // UI组件
    QVBoxLayout *m_mainLayout;
    QGroupBox *m_filterGroup;
    QHBoxLayout *m_filterLayout;
    
    // 筛选控件
    QComboBox *m_statusFilter;
    QComboBox *m_typeFilter;
    QComboBox *m_priorityFilter;
    QLineEdit *m_searchEdit;
    QPushButton *m_refreshBtn;
    
    // 操作按钮
    QPushButton *m_createBtn;
    QPushButton *m_editBtn;
    QPushButton *m_viewBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_closeBtn;
    
    // 状态转换按钮
    QPushButton *m_assignBtn;      // 派发
    QPushButton *m_startBtn;        // 开始
    QPushButton *m_completeBtn;     // 完成
    QPushButton *m_cancelBtn;       // 取消
    
    // 工单列表
    QTableWidget *m_tableWidget;
    
    // 数据
    QVector<WorkOrder> m_workOrders;
    int m_currentSelectedRow;
    
    // 状态转换管理器
    WorkOrderStatusTransition *m_statusTransition;
    
    // 辅助方法
    void updateStatusTransitionButtons();
    bool performStatusTransition(const QString &targetStatus);
};

#endif // WORKORDERMANAGERDIALOG_H

