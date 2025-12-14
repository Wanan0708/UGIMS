#include "profiledialog.h"
#include "core/auth/sessionmanager.h"
#include "core/models/user.h"
#include "dao/userdao.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

ProfileDialog::ProfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("个人信息");
    setMinimumSize(500, 600);
    setModal(true);
    
    setupUI();
    loadUserInfo();
}

ProfileDialog::~ProfileDialog()
{
}

void ProfileDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // ========== 基本信息组 ==========
    m_basicGroup = new QGroupBox("基本信息", this);
    QFormLayout *basicLayout = new QFormLayout(m_basicGroup);
    basicLayout->setSpacing(10);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setReadOnly(true);
    m_usernameEdit->setStyleSheet("background-color: #f0f0f0;");
    basicLayout->addRow("用户名:", m_usernameEdit);
    
    m_realNameEdit = new QLineEdit(this);
    basicLayout->addRow("真实姓名:", m_realNameEdit);
    
    m_emailEdit = new QLineEdit(this);
    // 邮箱格式验证
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    m_emailEdit->setValidator(new QRegularExpressionValidator(emailRegex, this));
    basicLayout->addRow("邮箱:", m_emailEdit);
    
    m_phoneEdit = new QLineEdit(this);
    // 电话格式验证（支持11位手机号）
    QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
    m_phoneEdit->setValidator(new QRegularExpressionValidator(phoneRegex, this));
    basicLayout->addRow("电话:", m_phoneEdit);
    
    m_departmentEdit = new QLineEdit(this);
    basicLayout->addRow("部门:", m_departmentEdit);
    
    m_organizationEdit = new QLineEdit(this);
    basicLayout->addRow("组织:", m_organizationEdit);
    
    m_basicGroup->setLayout(basicLayout);
    mainLayout->addWidget(m_basicGroup);
    
    // ========== 账户信息组 ==========
    m_accountGroup = new QGroupBox("账户信息", this);
    QFormLayout *accountLayout = new QFormLayout(m_accountGroup);
    accountLayout->setSpacing(10);
    
    m_roleLabel = new QLabel(this);
    m_roleLabel->setStyleSheet("color: #666;");
    accountLayout->addRow("角色:", m_roleLabel);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #666;");
    accountLayout->addRow("状态:", m_statusLabel);
    
    m_createdAtLabel = new QLabel(this);
    m_createdAtLabel->setStyleSheet("color: #666;");
    accountLayout->addRow("创建时间:", m_createdAtLabel);
    
    m_lastLoginLabel = new QLabel(this);
    m_lastLoginLabel->setStyleSheet("color: #666;");
    accountLayout->addRow("最后登录:", m_lastLoginLabel);
    
    m_accountGroup->setLayout(accountLayout);
    mainLayout->addWidget(m_accountGroup);
    
    // ========== 密码修改组 ==========
    m_passwordGroup = new QGroupBox("修改密码", this);
    QFormLayout *passwordLayout = new QFormLayout(m_passwordGroup);
    passwordLayout->setSpacing(10);
    
    m_oldPasswordEdit = new QLineEdit(this);
    m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addRow("当前密码:", m_oldPasswordEdit);
    
    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addRow("新密码:", m_newPasswordEdit);
    
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addRow("确认密码:", m_confirmPasswordEdit);
    
    m_changePasswordBtn = new QPushButton("修改密码", this);
    passwordLayout->addRow("", m_changePasswordBtn);
    
    m_passwordGroup->setLayout(passwordLayout);
    mainLayout->addWidget(m_passwordGroup);
    
    // ========== 权限信息组 ==========
    m_permissionGroup = new QGroupBox("权限信息", this);
    QVBoxLayout *permissionLayout = new QVBoxLayout(m_permissionGroup);
    
    m_permissionsLabel = new QLabel(this);
    m_permissionsLabel->setWordWrap(true);
    m_permissionsLabel->setStyleSheet("color: #666;");
    permissionLayout->addWidget(m_permissionsLabel);
    
    m_permissionGroup->setLayout(permissionLayout);
    mainLayout->addWidget(m_permissionGroup);
    
    mainLayout->addStretch();
    
    // ========== 按钮 ==========
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveBtn = new QPushButton("保存", this));
    buttonLayout->addWidget(m_cancelBtn = new QPushButton("取消", this));
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ProfileDialog::onCancelClicked);
    connect(m_changePasswordBtn, &QPushButton::clicked, this, &ProfileDialog::onChangePasswordClicked);
}

void ProfileDialog::loadUserInfo()
{
    User currentUser = SessionManager::instance().currentUser();
    if (!currentUser.isValid()) {
        QMessageBox::warning(this, "错误", "无法获取当前用户信息");
        return;
    }
    
    // 基本信息
    m_usernameEdit->setText(currentUser.username());
    m_realNameEdit->setText(currentUser.realName());
    m_emailEdit->setText(currentUser.email());
    m_phoneEdit->setText(currentUser.phone());
    m_departmentEdit->setText(currentUser.department());
    m_organizationEdit->setText(currentUser.organization());
    
    // 账户信息（只读）
    m_roleLabel->setText(User::getRoleDisplayName(currentUser.role()));
    m_statusLabel->setText(User::getStatusDisplayName(currentUser.status()));
    
    if (currentUser.createdAt().isValid()) {
        m_createdAtLabel->setText(currentUser.createdAt().toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        m_createdAtLabel->setText("未知");
    }
    
    if (currentUser.lastLogin().isValid()) {
        m_lastLoginLabel->setText(currentUser.lastLogin().toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        m_lastLoginLabel->setText("从未登录");
    }
    
    // 权限信息
    updatePermissionDisplay();
}

void ProfileDialog::updatePermissionDisplay()
{
    User currentUser = SessionManager::instance().currentUser();
    if (!currentUser.isValid()) {
        return;
    }
    
    QStringList permissions = currentUser.permissions();
    if (permissions.isEmpty()) {
        m_permissionsLabel->setText("无特殊权限");
    } else {
        QString permissionText = permissions.join("、");
        m_permissionsLabel->setText(permissionText);
    }
}

void ProfileDialog::onSaveClicked()
{
    if (!validateInput()) {
        return;
    }
    
    saveUserInfo();
}

void ProfileDialog::onCancelClicked()
{
    close();
}

void ProfileDialog::onChangePasswordClicked()
{
    QString oldPassword = m_oldPasswordEdit->text();
    QString newPassword = m_newPasswordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    
    if (oldPassword.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写完整的密码信息");
        return;
    }
    
    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "错误", "新密码和确认密码不一致");
        return;
    }
    
    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "错误", "密码长度至少为6位");
        return;
    }
    
    if (changePassword(oldPassword, newPassword)) {
        QMessageBox::information(this, "成功", "密码修改成功");
        m_oldPasswordEdit->clear();
        m_newPasswordEdit->clear();
        m_confirmPasswordEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "密码修改失败，请检查当前密码是否正确");
    }
}

bool ProfileDialog::validateInput()
{
    // 验证邮箱格式
    if (!m_emailEdit->text().isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(m_emailEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "验证失败", "邮箱格式不正确");
            m_emailEdit->setFocus();
            return false;
        }
    }
    
    // 验证电话格式
    if (!m_phoneEdit->text().isEmpty()) {
        QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
        if (!phoneRegex.match(m_phoneEdit->text()).hasMatch()) {
            QMessageBox::warning(this, "验证失败", "电话格式不正确，请输入11位手机号");
            m_phoneEdit->setFocus();
            return false;
        }
    }
    
    return true;
}

void ProfileDialog::saveUserInfo()
{
    User currentUser = SessionManager::instance().currentUser();
    if (!currentUser.isValid()) {
        QMessageBox::warning(this, "错误", "无法获取当前用户信息");
        return;
    }
    
    // 更新用户信息
    currentUser.setRealName(m_realNameEdit->text());
    currentUser.setEmail(m_emailEdit->text());
    currentUser.setPhone(m_phoneEdit->text());
    currentUser.setDepartment(m_departmentEdit->text());
    currentUser.setOrganization(m_organizationEdit->text());
    
    // 保存到数据库
    UserDAO dao;
    if (dao.updateUser(currentUser)) {
        QMessageBox::information(this, "成功", "个人信息已保存");
        close();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请稍后重试");
    }
}

bool ProfileDialog::changePassword(const QString &oldPassword, const QString &newPassword)
{
    User currentUser = SessionManager::instance().currentUser();
    if (!currentUser.isValid()) {
        return false;
    }
    
    // 验证当前密码
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(oldPassword.toUtf8());
    QString oldPasswordHash = hash.result().toHex();
    
    if (oldPasswordHash != currentUser.passwordHash()) {
        return false;
    }
    
    // 生成新密码哈希
    QCryptographicHash newHash(QCryptographicHash::Sha256);
    newHash.addData(newPassword.toUtf8());
    QString newPasswordHash = newHash.result().toHex();
    
    // 更新密码
    currentUser.setPasswordHash(newPasswordHash);
    
    // 保存到数据库
    UserDAO dao;
    return dao.updateUser(currentUser);
}

