#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFontComboBox>

/**
 * @brief 系统设置对话框
 * 只包含软件本身的设置，不包含业务功能相关的设置
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onApplyClicked();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

private:
    QTabWidget *m_tabWidget;
    
    // 通用设置
    QComboBox *m_languageCombo;
    QComboBox *m_themeCombo;
    QCheckBox *m_autosaveCheck;
    QSpinBox *m_autosaveIntervalSpin;
    QCheckBox *m_restoreWindowCheck;
    QSpinBox *m_windowWidthSpin;
    QSpinBox *m_windowHeightSpin;
    
    // 界面设置
    QFontComboBox *m_fontFamilyCombo;
    QSpinBox *m_fontSizeSpin;
    QCheckBox *m_animationCheck;
    QSpinBox *m_animationDurationSpin;
    
    // 日志设置
    QCheckBox *m_enableLoggingCheck;
    QComboBox *m_logLevelCombo;
    QLineEdit *m_logDirEdit;
    QPushButton *m_browseLogDirBtn;
    QSpinBox *m_logMaxSizeSpin;
    QSpinBox *m_logBackupCountSpin;
    
    // 性能设置
    QSpinBox *m_maxCacheSizeSpin;
    QCheckBox *m_enableHardwareAccelCheck;
    
    // 按钮
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_applyBtn;
};

#endif // SETTINGSDIALOG_H

