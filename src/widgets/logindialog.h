#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

// 前置声明
class QString;
class QLineEdit;
class QComboBox;
class CustomComboBox;
class QPushButton;
class QLabel;
class QCheckBox;
class QWidget;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QPoint;

/**
 * @brief 登录对话框
 * 提供用户登录功能，支持记住密码
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    // 获取登录的用户名
    QString username() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onLoginClicked();
    void onCancelClicked();
    void onUsernameChanged();
    void onUsernameComboBoxActivated(int index);
    void onPasswordChanged();

private:
    void setupUI();
    void updateLoginButton();
    void updateContentWidgetMask();
    void loadSavedCredentials();
    void loadPasswordForUsername(const QString &username);
    void saveCredentialsForUsername(const QString &username);
    void clearSavedCredentialsForUsername(const QString &username);
    QStringList getSavedUsernames();
    QString encryptPassword(const QString &password);
    QString decryptPassword(const QString &encryptedPassword);

private:
    QWidget *m_contentWidget;
    QWidget *m_titleBar;
    QLabel *m_iconLabel;
    QPushButton *m_closeBtn;
    CustomComboBox *m_usernameComboBox;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_cancelBtn;
    QCheckBox *m_rememberCheckBox;
    QLabel *m_errorLabel;
    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;
    
    QString m_username;
    
    // 用于窗口拖动
    QPoint m_dragPosition;
    bool m_dragging;
};

#endif // LOGINDIALOG_H

