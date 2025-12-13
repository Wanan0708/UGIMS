#include "logindialog.h"
#include "customcombobox.h"
#include "core/auth/sessionmanager.h"
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QApplication>
#include <QFont>
#include <QCryptographicHash>
#include <QByteArray>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QIcon>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractItemView>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <QPoint>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_dragging(false)
{
    setWindowTitle("用户登录");
    setModal(true);
    // 更紧凑的尺寸，简约设计
    setFixedSize(420, 520);
    
    // 设置窗口标志 - 保持透明背景以便只显示卡片
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUI();
    loadSavedCredentials();
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::setupUI()
{
    // 创建内容容器（用于圆角和阴影效果）
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("contentWidget");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_contentWidget);
    
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建标题栏
    m_titleBar = new QWidget(m_contentWidget);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(50);
    m_titleBar->setStyleSheet(
        "#titleBar {"
        "    background-color: transparent;"
        "    border-top-left-radius: 16px;"
        "    border-top-right-radius: 16px;"
        "}"
    );
    
    QHBoxLayout *titleBarLayout = new QHBoxLayout(m_titleBar);
    titleBarLayout->setContentsMargins(10, 0, 10, 0);
    titleBarLayout->setSpacing(10);
    
    // 左上角图标
    m_iconLabel = new QLabel(m_titleBar);
    m_iconLabel->setFixedSize(32, 32);  // 固定正方形尺寸，防止被挤扁
    m_iconLabel->setAlignment(Qt::AlignCenter);
    
    QIcon appIcon = QApplication::windowIcon();
    if (appIcon.isNull()) {
        // 如果没有设置图标，使用默认图标或文字
        m_iconLabel->setText("UG");
        m_iconLabel->setStyleSheet(
            "QLabel {"
            "    background-color: #3498db;"
            "    color: white;"
            "    border-radius: 20px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
        );
    } else {
        // 获取原始图标尺寸，保持宽高比
        QPixmap iconPixmap = appIcon.pixmap(32, 32);
        // 使用 scaled 方法保持宽高比，而不是 setScaledContents
        if (!iconPixmap.isNull()) {
            iconPixmap = iconPixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_iconLabel->setPixmap(iconPixmap);
        }
    }
    titleBarLayout->addWidget(m_iconLabel);
    
    titleBarLayout->addStretch();
    
    // 右上角关闭按钮
    m_closeBtn = new QPushButton(m_titleBar);
    m_closeBtn->setFixedSize(32, 32);
    m_closeBtn->setText("×");
    m_closeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 16px;"
        "    color: #7f8c8d;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0392b;"
        "}"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    titleBarLayout->addWidget(m_closeBtn);
    
    contentLayout->addWidget(m_titleBar);
    
    // 内容区域
    QWidget *bodyWidget = new QWidget(m_contentWidget);
    bodyWidget->setObjectName("bodyWidget");
    QVBoxLayout *bodyLayout = new QVBoxLayout(bodyWidget);
    bodyLayout->setSpacing(25);
    bodyLayout->setContentsMargins(50, 30, 50, 40);
    
    // 标题区域
    QVBoxLayout *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(8);
    
    m_titleLabel = new QLabel("UGIMS", bodyWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setFamily("Microsoft YaHei UI");
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #2c3e50;");
    
    m_subtitleLabel = new QLabel("城市地下管网智能管理系统", bodyWidget);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    QFont subtitleFont = m_subtitleLabel->font();
    subtitleFont.setPointSize(11);
    subtitleFont.setFamily("Microsoft YaHei UI");
    m_subtitleLabel->setFont(subtitleFont);
    m_subtitleLabel->setStyleSheet("color: #7f8c8d;");
    
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_subtitleLabel);
    bodyLayout->addLayout(titleLayout);
    
    // 表单区域
    QVBoxLayout *formLayout = new QVBoxLayout;
    formLayout->setSpacing(20);
    
    // 用户名输入框（改为 ComboBox）
    QVBoxLayout *usernameLayout = new QVBoxLayout;
    usernameLayout->setSpacing(8);
    QLabel *usernameLabel = new QLabel("用户名", bodyWidget);
    usernameLabel->setStyleSheet("color: #34495e; font-size: 13px; font-weight: 500;");
    usernameLayout->addWidget(usernameLabel);
    
    m_usernameComboBox = new CustomComboBox(bodyWidget);
    m_usernameComboBox->setEditable(true);
    m_usernameComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_usernameComboBox->lineEdit()->setPlaceholderText("请输入用户名或选择已有账号");
    m_usernameComboBox->lineEdit()->setMaxLength(50);
    m_usernameComboBox->setMinimumHeight(45);
    
    m_usernameComboBox->setStyleSheet(
        "QComboBox {"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    padding-right: 40px;"
        "    font-size: 14px;"
        "    background-color: #ffffff;"
        "    color: #2c3e50;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #f8f9fa;"
        "}"
        "QComboBox:hover {"
        "    border: 2px solid #bdc3c7;"
        "}"
        "QComboBox::drop-down {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: right center;"
        "    border: none;"
        "    width: 32px;"
        "    background-color: transparent;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    width: 0;"
        "    height: 0;"
        "}"
        "QComboBox QAbstractItemView {"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background-color: #ffffff;"
        "    selection-background-color: #3498db;"
        "    selection-color: #ffffff;"
        "    padding: 3px;"
        "    outline: none;"
        "    min-width: 200px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    height: 26px;"
        "    padding: 4px 16px;"
        "    border-radius: 6px;"
        "    margin: 1px 0px;"
        "    color: #2c3e50;"
        "    font-size: 14px;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "    background-color: #f8f9fa;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "    background-color: #3498db;"
        "    color: #ffffff;"
        "}"
    );
    
    // 通过view直接设置item高度，确保生效
    QAbstractItemView *view = m_usernameComboBox->view();
    if (view) {
        view->setStyleSheet(
            "QAbstractItemView::item {"
            "    height: 26px;"
            "    min-height: 26px;"
            "    max-height: 26px;"
            "}"
        );
    }
    usernameLayout->addWidget(m_usernameComboBox);
    formLayout->addLayout(usernameLayout);
    
    // 密码输入框
    QVBoxLayout *passwordLayout = new QVBoxLayout;
    passwordLayout->setSpacing(8);
    QLabel *passwordLabel = new QLabel("密码", bodyWidget);
    passwordLabel->setStyleSheet("color: #34495e; font-size: 13px; font-weight: 500;");
    passwordLayout->addWidget(passwordLabel);
    
    m_passwordEdit = new QLineEdit(bodyWidget);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMaxLength(100);
    m_passwordEdit->setMinimumHeight(45);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    font-size: 14px;"
        "    background-color: #ffffff;"
        "    color: #2c3e50;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #f8f9fa;"
        "}"
        "QLineEdit:hover {"
        "    border: 2px solid #bdc3c7;"
        "}"
    );
    passwordLayout->addWidget(m_passwordEdit);
    formLayout->addLayout(passwordLayout);
    
    bodyLayout->addLayout(formLayout);
    
    // 记住密码选项
    m_rememberCheckBox = new QCheckBox("记住密码", bodyWidget);
    m_rememberCheckBox->setStyleSheet(
        "QCheckBox {"
        "    color: #34495e;"
        "    font-size: 13px;"
        "    spacing: 8px;"
        "}"
        "QCheckBox::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #3498db;"
        "    border: 2px solid #3498db;"
        "    image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iOSIgdmlld0JveD0iMCAwIDEyIDkiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxwYXRoIGQ9Ik0xIDQuNUw0LjUgOEwxMSAxIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgo8L3N2Zz4=);"
        "}"
        "QCheckBox::indicator:hover {"
        "    border: 2px solid #3498db;"
        "}"
    );
    bodyLayout->addWidget(m_rememberCheckBox);
    
    // 错误提示标签
    m_errorLabel = new QLabel(bodyWidget);
    m_errorLabel->setStyleSheet(
        "color: #e74c3c;"
        "font-size: 12px;"
        "padding: 8px;"
        "background-color: #fee;"
        "border-radius: 6px;"
        "border: 1px solid #fcc;"
    );
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    bodyLayout->addWidget(m_errorLabel);
    
    // 按钮区域
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->setSpacing(12);
    
    m_loginBtn = new QPushButton("登录", bodyWidget);
    m_loginBtn->setDefault(true);
    m_loginBtn->setEnabled(false);
    m_loginBtn->setMinimumHeight(50);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3498db, stop:1 #2980b9);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 12px;"
        "}"
        "QPushButton:hover:enabled {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2980b9, stop:1 #1f6391);"
        "}"
        "QPushButton:pressed:enabled {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1f6391, stop:1 #1a5276);"
        "}"
        "QPushButton:disabled {"
        "    background: #bdc3c7;"
        "    color: #ecf0f1;"
        "}"
    );
    buttonLayout->addWidget(m_loginBtn);
    
    m_cancelBtn = new QPushButton("取消", bodyWidget);
    m_cancelBtn->setMinimumHeight(45);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #ecf0f1;"
        "    color: #34495e;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    padding: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d5dbdb;"
        "    border: 2px solid #95a5a6;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #bdc3c7;"
        "}"
    );
    buttonLayout->addWidget(m_cancelBtn);
    
    bodyLayout->addLayout(buttonLayout);
    bodyLayout->addStretch();
    
    // 将bodyWidget添加到contentLayout
    contentLayout->addWidget(bodyWidget);
    
    // 设置内容窗口样式
    m_contentWidget->setStyleSheet(
        "#contentWidget {"
        "    background-color: #ffffff;"
        "    border-radius: 16px;"
        "}"
    );
    
    // 使用遮罩来裁剪圆角，去除圆角外的虚影
    updateContentWidgetMask();
    
    // 连接信号
    connect(m_usernameComboBox->lineEdit(), &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    connect(m_usernameComboBox, QOverload<int>::of(&QComboBox::activated), 
            this, &LoginDialog::onUsernameComboBoxActivated);
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    
    // 设置焦点
    if (m_usernameComboBox->currentText().isEmpty()) {
        m_usernameComboBox->setFocus();
    } else {
        m_passwordEdit->setFocus();
    }
}

void LoginDialog::onUsernameChanged()
{
    QString currentText = m_usernameComboBox->currentText().trimmed();
    
    // 如果用户手动输入的新账号不在列表中，清空密码和取消记住密码
    if (!currentText.isEmpty() && m_usernameComboBox->findText(currentText) == -1) {
        // 这是一个新账号，清空密码和记住密码状态
        m_passwordEdit->clear();
        m_rememberCheckBox->setChecked(false);
    }
    
    updateLoginButton();
    m_errorLabel->hide();
}

void LoginDialog::onUsernameComboBoxActivated(int index)
{
    // 当从下拉列表选择账号时，自动加载该账号的密码
    if (index >= 0) {
        QString username = m_usernameComboBox->itemText(index);
        if (!username.isEmpty()) {
            loadPasswordForUsername(username);
        }
    }
    updateLoginButton();
    m_errorLabel->hide();
}

void LoginDialog::onPasswordChanged()
{
    updateLoginButton();
    m_errorLabel->hide();
}

void LoginDialog::updateLoginButton()
{
    bool canLogin = !m_usernameComboBox->currentText().trimmed().isEmpty() &&
                    !m_passwordEdit->text().isEmpty();
    m_loginBtn->setEnabled(canLogin);
}

QString LoginDialog::username() const
{
    return m_username;
}

void LoginDialog::onLoginClicked()
{
    QString username = m_usernameComboBox->currentText().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText("请输入用户名和密码");
        m_errorLabel->show();
        return;
    }
    
    // 尝试登录
    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("登录中...");
    
    bool success = SessionManager::instance().login(username, password);
    
    if (success) {
        m_username = username;
        
        // 保存或清除凭据（按账号分别保存）
        if (m_rememberCheckBox->isChecked()) {
            saveCredentialsForUsername(username);
        } else {
            clearSavedCredentialsForUsername(username);
        }
        
        accept();  // 关闭对话框并返回Accepted
    } else {
        m_errorLabel->setText("用户名或密码错误，或账户已被禁用");
        m_errorLabel->show();
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        m_loginBtn->setEnabled(true);
        m_loginBtn->setText("登录");
    }
}

void LoginDialog::onCancelClicked()
{
    reject();  // 关闭对话框并返回Rejected
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    // 不绘制背景，只显示白色卡片
    QDialog::paintEvent(event);
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // 窗口大小改变时更新遮罩
    if (m_contentWidget) {
        updateContentWidgetMask();
    }
}

void LoginDialog::updateContentWidgetMask()
{
    if (!m_contentWidget) {
        return;
    }
    
    // 使用遮罩来裁剪圆角，去除圆角外的虚影
    QPixmap maskPixmap(m_contentWidget->size());
    maskPixmap.fill(Qt::transparent);
    QPainter maskPainter(&maskPixmap);
    maskPainter.setRenderHint(QPainter::Antialiasing);
    maskPainter.setBrush(Qt::black);
    maskPainter.setPen(Qt::NoPen);
    maskPainter.drawRoundedRect(maskPixmap.rect(), 16, 16);
    maskPainter.end();
    m_contentWidget->setMask(maskPixmap.mask());
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在标题栏区域
        if (m_titleBar) {
            // 将事件坐标转换为相对于m_contentWidget的坐标，再转换为相对于m_titleBar的坐标
            QPoint posInContent = m_contentWidget->mapFromParent(event->pos());
            QPoint posInTitleBar = m_titleBar->mapFromParent(posInContent);
            if (m_titleBar->rect().contains(posInTitleBar)) {
                m_dragging = true;
                m_dragPosition = event->globalPos() - frameGeometry().topLeft();
                event->accept();
                return;
            }
        }
    }
    QDialog::mousePressEvent(event);
}

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
    QDialog::mouseMoveEvent(event);
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
    QDialog::mouseReleaseEvent(event);
}

void LoginDialog::loadSavedCredentials()
{
    // 加载所有已保存的账号到 ComboBox
    QStringList savedUsernames = getSavedUsernames();
    m_usernameComboBox->addItems(savedUsernames);
    
    // 如果有保存的账号，加载最后一个登录的账号
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("Login");
    QString lastUsername = settings.value("lastUsername", "").toString();
    settings.endGroup();
    
    if (!lastUsername.isEmpty() && savedUsernames.contains(lastUsername)) {
        m_usernameComboBox->setCurrentText(lastUsername);
        loadPasswordForUsername(lastUsername);
    } else if (!savedUsernames.isEmpty()) {
        // 如果没有最后登录的账号，加载第一个保存的账号
        m_usernameComboBox->setCurrentIndex(0);
        loadPasswordForUsername(savedUsernames.first());
    }
}

void LoginDialog::loadPasswordForUsername(const QString &username)
{
    if (username.isEmpty()) {
        return;
    }
    
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("Login");
    settings.beginGroup("Accounts");
    settings.beginGroup(username);
    
    bool rememberPassword = settings.value("rememberPassword", false).toBool();
    QString encryptedPassword = settings.value("password", "").toString();
    
    if (rememberPassword && !encryptedPassword.isEmpty()) {
        m_passwordEdit->setText(decryptPassword(encryptedPassword));
        m_rememberCheckBox->setChecked(true);
    } else {
        m_passwordEdit->clear();
        m_rememberCheckBox->setChecked(false);
    }
    
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
}

QStringList LoginDialog::getSavedUsernames()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("Login");
    settings.beginGroup("Accounts");
    
    QStringList usernames;
    
    // 获取所有账号组
    QStringList groups = settings.childGroups();
    for (const QString &group : groups) {
        settings.beginGroup(group);
        bool rememberPassword = settings.value("rememberPassword", false).toBool();
        if (rememberPassword) {
            usernames.append(group);
        }
        settings.endGroup();
    }
    
    settings.endGroup();
    settings.endGroup();
    
    return usernames;
}

void LoginDialog::saveCredentialsForUsername(const QString &username)
{
    if (username.isEmpty()) {
        return;
    }
    
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("Login");
    
    // 保存最后登录的账号
    settings.setValue("lastUsername", username);
    
    // 保存该账号的密码和记住密码状态
    settings.beginGroup("Accounts");
    settings.beginGroup(username);
    
    QString password = m_passwordEdit->text();
    settings.setValue("rememberPassword", true);
    settings.setValue("password", encryptPassword(password));
    
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
    settings.sync();
    
    // 如果账号不在 ComboBox 中，添加到列表
    if (m_usernameComboBox->findText(username) == -1) {
        m_usernameComboBox->addItem(username);
    }
}

void LoginDialog::clearSavedCredentialsForUsername(const QString &username)
{
    if (username.isEmpty()) {
        return;
    }
    
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("Login");
    settings.beginGroup("Accounts");
    settings.beginGroup(username);
    
    // 清除该账号的保存信息
    settings.remove("rememberPassword");
    settings.remove("password");
    
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
    settings.sync();
    
    // 从 ComboBox 中移除该账号
    int index = m_usernameComboBox->findText(username);
    if (index != -1) {
        m_usernameComboBox->removeItem(index);
    }
}

QString LoginDialog::encryptPassword(const QString &password)
{
    // 使用简单的XOR加密（可逆）
    // 注意：这不是强加密，仅用于本地存储的便利性
    // 实际生产环境应使用AES等对称加密算法
    QByteArray data = password.toUtf8();
    QString key = QApplication::applicationName() + "UGIMS2024SecretKey";
    QByteArray keyBytes = key.toUtf8();
    QByteArray encrypted;
    
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data[i] ^ keyBytes[i % keyBytes.size()]);
    }
    
    // Base64编码以便存储
    return encrypted.toBase64();
}

QString LoginDialog::decryptPassword(const QString &encryptedPassword)
{
    // 解码Base64
    QByteArray data = QByteArray::fromBase64(encryptedPassword.toUtf8());
    
    // XOR解密（与加密使用相同的密钥）
    QString key = QApplication::applicationName() + "UGIMS2024SecretKey";
    QByteArray keyBytes = key.toUtf8();
    QByteArray decrypted;
    
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data[i] ^ keyBytes[i % keyBytes.size()]);
    }
    
    return QString::fromUtf8(decrypted);
}

