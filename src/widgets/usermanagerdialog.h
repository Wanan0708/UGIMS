#ifndef USERMANAGERDIALOG_H
#define USERMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

class QVBoxLayout;
class QHBoxLayout;

/**
 * @brief 用户管理对话框
 * 提供用户的查看、创建、编辑、删除等功能
 */
class UserManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagerDialog(QWidget *parent = nullptr);
    ~UserManagerDialog();

private slots:
    void onCreateClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onTableSelectionChanged();
    void onFilterChanged();

private:
    void setupUI();
    void loadUsers();
    void refreshTable();
    void updateButtonStates();
    
    // 获取选中的用户ID
    int getSelectedUserId();

private:
    QTableWidget *m_tableWidget;
    QPushButton *m_createBtn;
    QPushButton *m_editBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_closeBtn;
    
    QLineEdit *m_usernameFilter;
    QComboBox *m_roleFilter;
    QComboBox *m_statusFilter;
};

#endif // USERMANAGERDIALOG_H

