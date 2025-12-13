#ifndef FACILITYEDITDIALOG_H
#define FACILITYEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QSpinBox>
#include "core/models/facility.h"

/**
 * @brief 设施属性编辑对话框
 * 
 * 用于创建或编辑设施的属性信息
 */
class FacilityEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FacilityEditDialog(QWidget *parent = nullptr, const Facility &facility = Facility());
    ~FacilityEditDialog();

    /**
     * @brief 获取设施对象
     */
    Facility resultFacility() const { return m_facility; }

private slots:
    void onSave();
    void onCancel();

private:
    void setupUI();
    void populateFields(const Facility &facility);
    Facility collectInput() const;
    QString getTypeDisplayName(const QString &type) const;

private:
    Facility m_facility;

    // 基本信息
    QLineEdit *m_facilityIdEdit;
    QLineEdit *m_facilityNameEdit;
    QComboBox *m_typeCombo;
    
    // 位置信息
    QDoubleSpinBox *m_longitudeSpin;
    QDoubleSpinBox *m_latitudeSpin;
    QDoubleSpinBox *m_elevationSpin;
    
    // 物理属性
    QLineEdit *m_specEdit;
    QComboBox *m_materialCombo;
    
    // 关联信息
    QLineEdit *m_pipelineIdEdit;
    
    // 建设信息
    QDateEdit *m_buildDateEdit;
    QLineEdit *m_builderEdit;
    QLineEdit *m_ownerEdit;
    
    // 运维信息
    QComboBox *m_statusCombo;
    QSpinBox *m_healthScoreSpin;
    QDateEdit *m_lastInspectionEdit;
    QLineEdit *m_maintenanceUnitEdit;
    
    // 备注
    QTextEdit *m_remarksEdit;
    
    // 按钮
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // FACILITYEDITDIALOG_H

