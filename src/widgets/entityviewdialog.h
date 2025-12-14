#ifndef ENTITYVIEWDIALOG_H
#define ENTITYVIEWDIALOG_H

#include <QDialog>

class Pipeline;
class Facility;
class QVBoxLayout;
class QScrollArea;
class QWidget;
class QPushButton;

/**
 * @brief 实体属性查看对话框
 * 用于查看管线和设施的详细信息（只读模式）
 */
class EntityViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EntityViewDialog(QWidget *parent = nullptr);
    ~EntityViewDialog();

    // 显示管线属性
    void setPipeline(const Pipeline &pipeline);
    
    // 显示设施属性
    void setFacility(const Facility &facility);

private slots:
    void onCloseClicked();
    void onEditClicked();

signals:
    void editRequested();

private:
    void setupUI();
    QWidget* createInfoCard(const QString &title, const QString &icon = "");
    void addInfoRow(QWidget *card, const QString &label, const QString &value, bool highlight = false, bool warning = false);
    void clearContent();

private:
    QVBoxLayout *m_mainLayout;
    QWidget *m_headerWidget;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QPushButton *m_closeBtn;
    QPushButton *m_editBtn;
    
    bool m_isPipeline;
};

#endif // ENTITYVIEWDIALOG_H

