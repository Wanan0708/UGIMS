#include "usermanagerdialog.h"
#include "dao/userdao.h"
#include "core/models/user.h"
#include "core/auth/permissionmanager.h"
#include "core/auth/sessionmanager.h"
#include "core/common/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

UserManagerDialog::UserManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("用户管理");
    setMinimumSize(800, 600);
    
    setupUI();
    loadUsers();
}

UserManagerDialog::~UserManagerDialog()
{
}

void UserManagerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 筛选面板
    QGroupBox *filterGroup = new QGroupBox("筛选条件", this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);
    
    filterLayout->addWidget(new QLabel("用户名:", this));
    m_usernameFilter = new QLineEdit(this);
    m_usernameFilter->setPlaceholderText("输入用户名搜索");
    filterLayout->addWidget(m_usernameFilter);
    
    filterLayout->addWidget(new QLabel("角色:", this));
    m_roleFilter = new QComboBox(this);
    m_roleFilter->addItem("全部", "");
    m_roleFilter->addItem("管理员", User::ROLE_ADMIN);
    m_roleFilter->addItem("管理者", User::ROLE_MANAGER);
    m_roleFilter->addItem("巡检员", User::ROLE_INSPECTOR);
    m_roleFilter->addItem("查看者", User::ROLE_VIEWER);
    filterLayout->addWidget(m_roleFilter);
    
    filterLayout->addWidget(new QLabel("状态:", this));
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItem("全部", "");
    m_statusFilter->addItem("激活", User::STATUS_ACTIVE);
    m_statusFilter->addItem("未激活", User::STATUS_INACTIVE);
    m_statusFilter->addItem("锁定", User::STATUS_LOCKED);
    filterLayout->addWidget(m_statusFilter);
    
    filterLayout->addStretch();
    mainLayout->addWidget(filterGroup);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createBtn = new QPushButton("新建用户", this);
    m_editBtn = new QPushButton("编辑", this);
    m_deleteBtn = new QPushButton("删除", this);
    m_refreshBtn = new QPushButton("刷新", this);
    m_closeBtn = new QPushButton("关闭", this);
    
    // 根据权限设置按钮状态
    m_createBtn->setEnabled(PermissionManager::canManageUsers());
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_createBtn);
    buttonLayout->addWidget(m_editBtn);
    buttonLayout->addWidget(m_deleteBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshBtn);
    buttonLayout->addWidget(m_closeBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    // 用户列表
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(8);
    QStringList headers = {"ID", "用户名", "真实姓名", "角色", "部门", "状态", "最后登录", "创建时间"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setAlternatingRowColors(true);
    
    mainLayout->addWidget(m_tableWidget);
    
    // 连接信号
    connect(m_createBtn, &QPushButton::clicked, this, &UserManagerDialog::onCreateClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &UserManagerDialog::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &UserManagerDialog::onDeleteClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &UserManagerDialog::onRefreshClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    connect(m_usernameFilter, &QLineEdit::textChanged, this, &UserManagerDialog::onFilterChanged);
    connect(m_roleFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &UserManagerDialog::onFilterChanged);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &UserManagerDialog::onFilterChanged);
    
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, 
            this, &UserManagerDialog::onTableSelectionChanged);
}

void UserManagerDialog::loadUsers()
{
    UserDAO dao;
    QVector<User> users = dao.findAll(10000);
    
    m_tableWidget->setRowCount(0);
    
    for (const User &user : users) {
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);
        
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(user.id())));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(user.username()));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(user.realName()));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(User::getRoleDisplayName(user.role())));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(user.department()));
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(User::getStatusDisplayName(user.status())));
        m_tableWidget->setItem(row, 6, new QTableWidgetItem(
            user.lastLogin().isValid() ? user.lastLogin().toString("yyyy-MM-dd hh:mm:ss") : ""));
        m_tableWidget->setItem(row, 7, new QTableWidgetItem(
            user.createdAt().isValid() ? user.createdAt().toString("yyyy-MM-dd hh:mm:ss") : ""));
    }
    
    updateButtonStates();
}

void UserManagerDialog::refreshTable()
{
    loadUsers();
}

void UserManagerDialog::onFilterChanged()
{
    // 简单的客户端过滤
    QString usernameFilter = m_usernameFilter->text().toLower();
    QString roleFilter = m_roleFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    
    for (int i = 0; i < m_tableWidget->rowCount(); ++i) {
        bool visible = true;
        
        if (!usernameFilter.isEmpty()) {
            QString username = m_tableWidget->item(i, 1)->text().toLower();
            if (!username.contains(usernameFilter)) {
                visible = false;
            }
        }
        
        if (!roleFilter.isEmpty() && visible) {
            UserDAO dao;
            int userId = m_tableWidget->item(i, 0)->text().toInt();
            User user = dao.findById(userId);
            if (user.role() != roleFilter) {
                visible = false;
            }
        }
        
        if (!statusFilter.isEmpty() && visible) {
            UserDAO dao;
            int userId = m_tableWidget->item(i, 0)->text().toInt();
            User user = dao.findById(userId);
            if (user.status() != statusFilter) {
                visible = false;
            }
        }
        
        m_tableWidget->setRowHidden(i, !visible);
    }
}

void UserManagerDialog::onTableSelectionChanged()
{
    updateButtonStates();
}

void UserManagerDialog::updateButtonStates()
{
    bool hasSelection = m_tableWidget->currentRow() >= 0;
    bool canManage = PermissionManager::canManageUsers();
    
    m_editBtn->setEnabled(hasSelection && canManage);
    m_deleteBtn->setEnabled(hasSelection && canManage);
}

int UserManagerDialog::getSelectedUserId()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        return 0;
    }
    return m_tableWidget->item(row, 0)->text().toInt();
}

void UserManagerDialog::onCreateClicked()
{
    // 简单的用户创建对话框
    bool ok;
    QString username = QInputDialog::getText(this, "新建用户", "用户名:", 
                                             QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) {
        return;
    }
    
    QString password = QInputDialog::getText(this, "新建用户", "密码:", 
                                            QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) {
        return;
    }
    
    QStringList roles = {"管理员", "管理者", "巡检员", "查看者"};
    QStringList roleValues = {User::ROLE_ADMIN, User::ROLE_MANAGER, 
                             User::ROLE_INSPECTOR, User::ROLE_VIEWER};
    QString roleName = QInputDialog::getItem(this, "新建用户", "角色:", 
                                             roles, 0, false, &ok);
    if (!ok) {
        return;
    }
    
    int roleIndex = roles.indexOf(roleName);
    QString role = roleValues[roleIndex];
    
    User newUser;
    newUser.setUsername(username);
    newUser.setRole(role);
    newUser.setStatus(User::STATUS_ACTIVE);
    
    UserDAO dao;
    if (dao.usernameExists(username)) {
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }
    
    if (dao.createUser(newUser, password)) {
        QMessageBox::information(this, "成功", "用户创建成功");
        loadUsers();
    } else {
        QMessageBox::warning(this, "错误", "用户创建失败");
    }
}

void UserManagerDialog::onEditClicked()
{
    int userId = getSelectedUserId();
    if (userId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的用户");
        return;
    }
    
    UserDAO dao;
    User user = dao.findById(userId);
    if (!user.isValid()) {
        QMessageBox::warning(this, "错误", "未找到该用户");
        return;
    }
    
    // 简单的编辑对话框
    bool ok;
    QString realName = QInputDialog::getText(this, "编辑用户", "真实姓名:", 
                                             QLineEdit::Normal, user.realName(), &ok);
    if (!ok) {
        return;
    }
    
    QStringList roles = {"管理员", "管理者", "巡检员", "查看者"};
    QStringList roleValues = {User::ROLE_ADMIN, User::ROLE_MANAGER, 
                             User::ROLE_INSPECTOR, User::ROLE_VIEWER};
    int currentRoleIndex = roleValues.indexOf(user.role());
    QString roleName = QInputDialog::getItem(this, "编辑用户", "角色:", 
                                             roles, currentRoleIndex, false, &ok);
    if (!ok) {
        return;
    }
    
    int roleIndex = roles.indexOf(roleName);
    QString role = roleValues[roleIndex];
    
    user.setRealName(realName);
    user.setRole(role);
    
    if (dao.updateUser(user)) {
        QMessageBox::information(this, "成功", "用户更新成功");
        loadUsers();
    } else {
        QMessageBox::warning(this, "错误", "用户更新失败");
    }
}

void UserManagerDialog::onDeleteClicked()
{
    int userId = getSelectedUserId();
    if (userId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的用户");
        return;
    }
    
    UserDAO dao;
    User user = dao.findById(userId);
    if (!user.isValid()) {
        QMessageBox::warning(this, "错误", "未找到该用户");
        return;
    }
    
    // 不能删除自己
    if (user.username() == SessionManager::instance().currentUsername()) {
        QMessageBox::warning(this, "错误", "不能删除当前登录用户");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认删除", 
                                     QString("确定要删除用户 %1 吗？").arg(user.username()),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (dao.deleteById(userId)) {
            QMessageBox::information(this, "成功", "用户删除成功");
            loadUsers();
        } else {
            QMessageBox::warning(this, "错误", "用户删除失败");
        }
    }
}

void UserManagerDialog::onRefreshClicked()
{
    loadUsers();
}

