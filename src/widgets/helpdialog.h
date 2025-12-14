#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QScrollArea;
class QWidget;

/**
 * @brief 帮助对话框
 * 提供系统使用帮助和功能说明
 */
class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog();

private slots:
    void onCloseClicked();

private:
    void setupUI();
    QWidget* createFeatureCard(const QString &icon, const QString &title, const QString &description);

private:
    QVBoxLayout *m_mainLayout;
    QWidget *m_headerWidget;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QPushButton *m_closeBtn;
};

#endif // HELPDIALOG_H

