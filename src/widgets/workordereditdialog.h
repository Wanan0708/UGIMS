#ifndef WORKORDEREDITDIALOG_H
#define WORKORDEREDITDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QPointF>
#include "core/models/workorder.h"

class QLineEdit;
class QTextEdit;
class QComboBox;
class QDateTimeEdit;
class QSpinBox;
class QPushButton;

/**
 * @brief 工单编辑对话框（新建/编辑）
 */
class WorkOrderEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkOrderEditDialog(QWidget *parent = nullptr, const WorkOrder &workOrder = WorkOrder());
    ~WorkOrderEditDialog() override = default;

    WorkOrder resultWorkOrder() const { return m_workOrder; }

private slots:
    void onSave();
    void onCancel();

private:
    void setupUI();
    void populateFields(const WorkOrder &workOrder);
    WorkOrder collectInput() const;

private:
    WorkOrder m_workOrder;

    // 基础信息
    QLineEdit *m_orderIdEdit;
    QLineEdit *m_titleEdit;
    QComboBox *m_typeCombo;
    QComboBox *m_priorityCombo;
    QComboBox *m_statusCombo;

    // 关联资产
    QComboBox *m_assetTypeCombo;
    QLineEdit *m_assetIdEdit;
    QLineEdit *m_lonEdit;
    QLineEdit *m_latEdit;

    // 内容与人员
    QTextEdit *m_descriptionEdit;
    QTextEdit *m_actionsEdit;
    QLineEdit *m_assignedToEdit;

    // 时间
    QDateTimeEdit *m_planStartEdit;
    QDateTimeEdit *m_planEndEdit;
    QDateTimeEdit *m_actualStartEdit;
    QDateTimeEdit *m_actualEndEdit;

    // 完成与审核
    QSpinBox *m_completionSpin;
    QTextEdit *m_resultEdit;
    QLineEdit *m_reviewedByEdit;
    QLineEdit *m_reviewResultEdit;
    QTextEdit *m_reviewCommentsEdit;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // WORKORDEREDITDIALOG_H

