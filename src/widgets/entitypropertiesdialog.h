#ifndef ENTITYPROPERTIESDIALOG_H
#define ENTITYPROPERTIESDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QGraphicsItem>

/**
 * @brief 实体属性编辑对话框
 * 支持管线和设施的属性查看和编辑
 */
class EntityPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    // 实体类型
    enum EntityType {
        Pipeline,   // 管线
        Facility    // 设施
    };

    explicit EntityPropertiesDialog(QGraphicsItem *item, 
                                   EntityType type,
                                   QWidget *parent = nullptr);
    ~EntityPropertiesDialog();

    // 获取编辑后的属性
    QString getName() const;
    QString getType() const;
    QString getDescription() const;
    QColor getColor() const;
    int getLineWidth() const;
    
signals:
    void propertiesChanged();
    void deleteRequested();

private slots:
    void onDeleteClicked();
    void onSaveClicked();

private:
    void setupUI();
    void loadProperties();
    void saveProperties();

private:
    QGraphicsItem *m_item;
    EntityType m_entityType;
    
    // UI控件
    QLineEdit *m_nameEdit;
    QComboBox *m_typeCombo;
    QTextEdit *m_descEdit;
    QComboBox *m_colorCombo;
    QSpinBox *m_lineWidthSpin;
    
    QPushButton *m_saveBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_cancelBtn;
};

#endif // ENTITYPROPERTIESDIALOG_H
