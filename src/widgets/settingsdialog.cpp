#include "widgets/settingsdialog.h"
#include "core/common/config.h"
#include "core/common/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("系统设置");
    setMinimumSize(600, 500);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
    
    // ========== 通用设置选项卡 ==========
    QWidget *generalTab = new QWidget();
    QVBoxLayout *generalMainLayout = new QVBoxLayout(generalTab);
    generalMainLayout->setSpacing(10);
    
    // 基本设置组
    QGroupBox *basicGroup = new QGroupBox("基本设置", this);
    QFormLayout *basicLayout = new QFormLayout(basicGroup);
    basicLayout->setSpacing(10);
    
    m_languageCombo = new QComboBox();
    m_languageCombo->addItems({"中文 (简体)", "English"});
    basicLayout->addRow("界面语言:", m_languageCombo);
    
    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"专业蓝调", "浅色主题", "深色主题"});
    basicLayout->addRow("主题样式:", m_themeCombo);
    
    basicGroup->setLayout(basicLayout);
    generalMainLayout->addWidget(basicGroup);
    
    // 自动保存组
    QGroupBox *autosaveGroup = new QGroupBox("自动保存", this);
    QFormLayout *autosaveLayout = new QFormLayout(autosaveGroup);
    autosaveLayout->setSpacing(10);
    
    m_autosaveCheck = new QCheckBox("启用自动保存");
    autosaveLayout->addRow("", m_autosaveCheck);
    
    m_autosaveIntervalSpin = new QSpinBox();
    m_autosaveIntervalSpin->setRange(60, 3600);
    m_autosaveIntervalSpin->setSuffix(" 秒");
    m_autosaveIntervalSpin->setEnabled(false);
    autosaveLayout->addRow("自动保存间隔:", m_autosaveIntervalSpin);
    
    connect(m_autosaveCheck, &QCheckBox::toggled, m_autosaveIntervalSpin, &QSpinBox::setEnabled);
    
    autosaveGroup->setLayout(autosaveLayout);
    generalMainLayout->addWidget(autosaveGroup);
    
    // 窗口设置组
    QGroupBox *windowGroup = new QGroupBox("窗口设置", this);
    QFormLayout *windowLayout = new QFormLayout(windowGroup);
    windowLayout->setSpacing(10);
    
    m_restoreWindowCheck = new QCheckBox("启动时恢复上次窗口大小和位置");
    windowLayout->addRow("", m_restoreWindowCheck);
    
    m_windowWidthSpin = new QSpinBox();
    m_windowWidthSpin->setRange(800, 4096);
    m_windowWidthSpin->setSuffix(" 像素");
    windowLayout->addRow("默认窗口宽度:", m_windowWidthSpin);
    
    m_windowHeightSpin = new QSpinBox();
    m_windowHeightSpin->setRange(600, 4096);
    m_windowHeightSpin->setSuffix(" 像素");
    windowLayout->addRow("默认窗口高度:", m_windowHeightSpin);
    
    windowGroup->setLayout(windowLayout);
    generalMainLayout->addWidget(windowGroup);
    
    generalMainLayout->addStretch();
    m_tabWidget->addTab(generalTab, "通用");
    
    // ========== 界面设置选项卡 ==========
    QWidget *uiTab = new QWidget();
    QVBoxLayout *uiMainLayout = new QVBoxLayout(uiTab);
    uiMainLayout->setSpacing(10);
    
    QGroupBox *fontGroup = new QGroupBox("字体设置", this);
    QFormLayout *fontLayout = new QFormLayout(fontGroup);
    fontLayout->setSpacing(10);
    
    m_fontFamilyCombo = new QFontComboBox();
    fontLayout->addRow("字体:", m_fontFamilyCombo);
    
    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setSuffix(" 磅");
    fontLayout->addRow("字号:", m_fontSizeSpin);
    
    fontGroup->setLayout(fontLayout);
    uiMainLayout->addWidget(fontGroup);
    
    QGroupBox *animationGroup = new QGroupBox("动画效果", this);
    QFormLayout *animationLayout = new QFormLayout(animationGroup);
    animationLayout->setSpacing(10);
    
    m_animationCheck = new QCheckBox("启用界面动画");
    animationLayout->addRow("", m_animationCheck);
    
    m_animationDurationSpin = new QSpinBox();
    m_animationDurationSpin->setRange(100, 1000);
    m_animationDurationSpin->setSuffix(" 毫秒");
    m_animationDurationSpin->setEnabled(false);
    animationLayout->addRow("动画时长:", m_animationDurationSpin);
    
    connect(m_animationCheck, &QCheckBox::toggled, m_animationDurationSpin, &QSpinBox::setEnabled);
    
    animationGroup->setLayout(animationLayout);
    uiMainLayout->addWidget(animationGroup);
    
    uiMainLayout->addStretch();
    m_tabWidget->addTab(uiTab, "界面");
    
    // ========== 日志设置选项卡 ==========
    QWidget *loggingTab = new QWidget();
    QVBoxLayout *loggingMainLayout = new QVBoxLayout(loggingTab);
    loggingMainLayout->setSpacing(10);
    
    QGroupBox *logGroup = new QGroupBox("日志配置", this);
    QFormLayout *logLayout = new QFormLayout(logGroup);
    logLayout->setSpacing(10);
    
    m_enableLoggingCheck = new QCheckBox("启用日志记录");
    logLayout->addRow("", m_enableLoggingCheck);
    
    m_logLevelCombo = new QComboBox();
    m_logLevelCombo->addItems({"调试", "信息", "警告", "错误", "严重"});
    logLayout->addRow("日志级别:", m_logLevelCombo);
    
    QHBoxLayout *logDirLayout = new QHBoxLayout();
    m_logDirEdit = new QLineEdit();
    m_browseLogDirBtn = new QPushButton("浏览...");
    logDirLayout->addWidget(m_logDirEdit);
    logDirLayout->addWidget(m_browseLogDirBtn);
    logLayout->addRow("日志目录:", logDirLayout);
    
    m_logMaxSizeSpin = new QSpinBox();
    m_logMaxSizeSpin->setRange(1, 1000);
    m_logMaxSizeSpin->setSuffix(" MB");
    logLayout->addRow("单个日志文件最大大小:", m_logMaxSizeSpin);
    
    m_logBackupCountSpin = new QSpinBox();
    m_logBackupCountSpin->setRange(1, 50);
    logLayout->addRow("日志备份文件数量:", m_logBackupCountSpin);
    
    connect(m_browseLogDirBtn, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择日志目录", m_logDirEdit->text());
        if (!dir.isEmpty()) {
            m_logDirEdit->setText(dir);
        }
    });
    
    logGroup->setLayout(logLayout);
    loggingMainLayout->addWidget(logGroup);
    
    loggingMainLayout->addStretch();
    m_tabWidget->addTab(loggingTab, "日志");
    
    // ========== 性能设置选项卡 ==========
    QWidget *performanceTab = new QWidget();
    QVBoxLayout *perfMainLayout = new QVBoxLayout(performanceTab);
    perfMainLayout->setSpacing(10);
    
    QGroupBox *cacheGroup = new QGroupBox("缓存设置", this);
    QFormLayout *cacheLayout = new QFormLayout(cacheGroup);
    cacheLayout->setSpacing(10);
    
    m_maxCacheSizeSpin = new QSpinBox();
    m_maxCacheSizeSpin->setRange(100, 100000);
    m_maxCacheSizeSpin->setSuffix(" MB");
    cacheLayout->addRow("系统缓存最大大小:", m_maxCacheSizeSpin);
    
    QLabel *cacheHint = new QLabel("提示: 地图缓存设置请在地图管理对话框中配置", this);
    cacheHint->setStyleSheet("color: #666; font-size: 11px;");
    cacheLayout->addRow("", cacheHint);
    
    cacheGroup->setLayout(cacheLayout);
    perfMainLayout->addWidget(cacheGroup);
    
    QGroupBox *hardwareGroup = new QGroupBox("硬件加速", this);
    QFormLayout *hardwareLayout = new QFormLayout(hardwareGroup);
    hardwareLayout->setSpacing(10);
    
    m_enableHardwareAccelCheck = new QCheckBox("启用硬件加速 (OpenGL)");
    hardwareLayout->addRow("", m_enableHardwareAccelCheck);
    
    QLabel *hardwareHint = new QLabel("提示: 启用硬件加速可提升地图渲染性能，但可能在某些显卡上出现兼容性问题", this);
    hardwareHint->setStyleSheet("color: #666; font-size: 11px;");
    hardwareHint->setWordWrap(true);
    hardwareLayout->addRow("", hardwareHint);
    
    hardwareGroup->setLayout(hardwareLayout);
    perfMainLayout->addWidget(hardwareGroup);
    
    perfMainLayout->addStretch();
    m_tabWidget->addTab(performanceTab, "性能");
    
    mainLayout->addWidget(m_tabWidget);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_saveBtn = new QPushButton("保存");
    m_cancelBtn = new QPushButton("取消");
    m_applyBtn = new QPushButton("应用");
    
    m_saveBtn->setMinimumWidth(80);
    m_cancelBtn->setMinimumWidth(80);
    m_applyBtn->setMinimumWidth(80);
    
    buttonLayout->addWidget(m_saveBtn);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_applyBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
}

void SettingsDialog::loadSettings()
{
    Config &config = Config::instance();
    
    // 通用设置
    QString language = config.getString("General/language", "zh_CN");
    m_languageCombo->setCurrentIndex(language == "zh_CN" ? 0 : 1);
    
    QString theme = config.getString("General/theme", "professional_blue");
    if (theme == "professional_blue") m_themeCombo->setCurrentIndex(0);
    else if (theme == "light") m_themeCombo->setCurrentIndex(1);
    else m_themeCombo->setCurrentIndex(2);
    
    m_autosaveCheck->setChecked(config.getBool("General/autosave", true));
    m_autosaveIntervalSpin->setValue(config.getInt("General/autosave_interval", 300));
    m_autosaveIntervalSpin->setEnabled(m_autosaveCheck->isChecked());
    
    m_restoreWindowCheck->setChecked(config.getBool("General/restore_window", true));
    m_windowWidthSpin->setValue(config.getInt("General/window_width", 1280));
    m_windowHeightSpin->setValue(config.getInt("General/window_height", 800));
    
    // 界面设置
    QString fontFamily = config.getString("UI/font_family", "Microsoft YaHei UI");
    m_fontFamilyCombo->setCurrentFont(QFont(fontFamily));
    m_fontSizeSpin->setValue(config.getInt("UI/font_size", 13));
    
    m_animationCheck->setChecked(config.getBool("UI/animation_enabled", true));
    m_animationDurationSpin->setValue(config.getInt("UI/animation_duration", 200));
    m_animationDurationSpin->setEnabled(m_animationCheck->isChecked());
    
    // 日志设置
    m_enableLoggingCheck->setChecked(config.getBool("Logging/enable_logging", true));
    
    QString logLevel = config.getString("Logging/log_level", "info");
    if (logLevel == "debug") m_logLevelCombo->setCurrentIndex(0);
    else if (logLevel == "info") m_logLevelCombo->setCurrentIndex(1);
    else if (logLevel == "warning") m_logLevelCombo->setCurrentIndex(2);
    else if (logLevel == "error") m_logLevelCombo->setCurrentIndex(3);
    else m_logLevelCombo->setCurrentIndex(4);
    
    m_logDirEdit->setText(config.getString("Logging/log_dir", "logs"));
    m_logMaxSizeSpin->setValue(config.getInt("Logging/log_max_size", 10485760) / 1048576); // 转换为MB
    m_logBackupCountSpin->setValue(config.getInt("Logging/log_backup_count", 5));
    
    // 性能设置
    m_maxCacheSizeSpin->setValue(config.getInt("Performance/max_cache_size", 5000));
    m_enableHardwareAccelCheck->setChecked(config.getBool("Performance/enable_hardware_accel", false));
}

void SettingsDialog::saveSettings()
{
    Config &config = Config::instance();
    
    // 通用设置
    config.setValue("General/language", m_languageCombo->currentIndex() == 0 ? "zh_CN" : "en_US");
    QString theme = m_themeCombo->currentIndex() == 0 ? "professional_blue" : 
                    (m_themeCombo->currentIndex() == 1 ? "light" : "dark");
    config.setValue("General/theme", theme);
    config.setValue("General/autosave", m_autosaveCheck->isChecked());
    config.setValue("General/autosave_interval", m_autosaveIntervalSpin->value());
    config.setValue("General/restore_window", m_restoreWindowCheck->isChecked());
    config.setValue("General/window_width", m_windowWidthSpin->value());
    config.setValue("General/window_height", m_windowHeightSpin->value());
    
    // 界面设置
    config.setValue("UI/font_family", m_fontFamilyCombo->currentFont().family());
    config.setValue("UI/font_size", m_fontSizeSpin->value());
    config.setValue("UI/animation_enabled", m_animationCheck->isChecked());
    config.setValue("UI/animation_duration", m_animationDurationSpin->value());
    
    // 日志设置
    config.setValue("Logging/enable_logging", m_enableLoggingCheck->isChecked());
    
    QString logLevel;
    switch (m_logLevelCombo->currentIndex()) {
        case 0: logLevel = "debug"; break;
        case 1: logLevel = "info"; break;
        case 2: logLevel = "warning"; break;
        case 3: logLevel = "error"; break;
        case 4: logLevel = "critical"; break;
        default: logLevel = "info"; break;
    }
    config.setValue("Logging/log_level", logLevel);
    config.setValue("Logging/log_dir", m_logDirEdit->text());
    config.setValue("Logging/log_max_size", m_logMaxSizeSpin->value() * 1048576); // 转换为字节
    config.setValue("Logging/log_backup_count", m_logBackupCountSpin->value());
    
    // 性能设置
    config.setValue("Performance/max_cache_size", m_maxCacheSizeSpin->value());
    config.setValue("Performance/enable_hardware_accel", m_enableHardwareAccelCheck->isChecked());
    
    // 保存到文件
    config.save();
    
    LOG_INFO("Settings saved");
}

void SettingsDialog::onSaveClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}

void SettingsDialog::onApplyClicked()
{
    saveSettings();
    QMessageBox::information(this, "提示", "设置已应用");
}

