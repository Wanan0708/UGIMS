#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

class QVBoxLayout;
class QFormLayout;

/**
 * @brief 个人信息对话框
 * 显示和编辑当前登录用户的信息
 */
class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);
    ~ProfileDialog();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onChangePasswordClicked();

private:
    void setupUI();
    void loadUserInfo();
    void saveUserInfo();
    bool changePassword(const QString &oldPassword, const QString &newPassword);
    bool validateInput();
    void updatePermissionDisplay();

private:
    // 基本信息
    QLineEdit *m_usernameEdit;      // 用户名（只读）
    QLineEdit *m_realNameEdit;      // 真实姓名
    QLineEdit *m_emailEdit;         // 邮箱
    QLineEdit *m_phoneEdit;         // 电话
    QLineEdit *m_departmentEdit;    // 部门
    QLineEdit *m_organizationEdit;  // 组织
    
    // 账户信息（只读）
    QLabel *m_roleLabel;           // 角色
    QLabel *m_statusLabel;          // 状态
    QLabel *m_createdAtLabel;       // 创建时间
    QLabel *m_lastLoginLabel;       // 最后登录
    
    // 密码修改
    QLineEdit *m_oldPasswordEdit;   // 当前密码
    QLineEdit *m_newPasswordEdit;   // 新密码
    QLineEdit *m_confirmPasswordEdit; // 确认密码
    QPushButton *m_changePasswordBtn;
    
    // 权限显示
    QLabel *m_permissionsLabel;     // 权限列表
    
    // 按钮
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    // 布局
    QGroupBox *m_basicGroup;
    QGroupBox *m_accountGroup;
    QGroupBox *m_passwordGroup;
    QGroupBox *m_permissionGroup;
};

#endif // PROFILEDIALOG_H

