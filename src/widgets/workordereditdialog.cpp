#include "workordereditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

WorkOrderEditDialog::WorkOrderEditDialog(QWidget *parent, const WorkOrder &workOrder)
    : QDialog(parent)
    , m_workOrder(workOrder)
{
    setWindowTitle(workOrder.isValid() ? tr("编辑工单") : tr("新建工单"));
    setMinimumSize(520, 640);
    setupUI();
    populateFields(workOrder);
}

void WorkOrderEditDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    m_orderIdEdit = new QLineEdit(this);
    m_orderIdEdit->setReadOnly(true);
    formLayout->addRow(tr("工单编号"), m_orderIdEdit);

    m_titleEdit = new QLineEdit(this);
    formLayout->addRow(tr("标题*"), m_titleEdit);  // 必填项标记

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(tr("日常巡检"), WorkOrder::TYPE_INSPECTION);
    m_typeCombo->addItem(tr("维修工单"), WorkOrder::TYPE_MAINTENANCE);
    m_typeCombo->addItem(tr("应急工单"), WorkOrder::TYPE_EMERGENCY);
    m_typeCombo->addItem(tr("改造工单"), WorkOrder::TYPE_RENOVATION);
    formLayout->addRow(tr("类型*"), m_typeCombo);  // 必填项标记

    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem(tr("低"), WorkOrder::PRIORITY_LOW);
    m_priorityCombo->addItem(tr("普通"), WorkOrder::PRIORITY_NORMAL);
    m_priorityCombo->addItem(tr("高"), WorkOrder::PRIORITY_HIGH);
    m_priorityCombo->addItem(tr("紧急"), WorkOrder::PRIORITY_URGENT);
    formLayout->addRow(tr("优先级"), m_priorityCombo);

    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem(tr("待分配"), WorkOrder::STATUS_PENDING);
    m_statusCombo->addItem(tr("已分配"), WorkOrder::STATUS_ASSIGNED);
    m_statusCombo->addItem(tr("进行中"), WorkOrder::STATUS_IN_PROGRESS);
    m_statusCombo->addItem(tr("已完成"), WorkOrder::STATUS_COMPLETED);
    m_statusCombo->addItem(tr("已取消"), WorkOrder::STATUS_CANCELLED);
    formLayout->addRow(tr("状态"), m_statusCombo);

    // 关联资产
    m_assetTypeCombo = new QComboBox(this);
    m_assetTypeCombo->addItem(tr("未指定"), "");
    m_assetTypeCombo->addItem(tr("管线"), "pipeline");
    m_assetTypeCombo->addItem(tr("设施"), "facility");
    formLayout->addRow(tr("资产类型"), m_assetTypeCombo);

    m_assetIdEdit = new QLineEdit(this);
    formLayout->addRow(tr("资产编号"), m_assetIdEdit);

    // 位置
    auto *lonLatLayout = new QHBoxLayout();
    m_lonEdit = new QLineEdit(this);
    m_lonEdit->setPlaceholderText(tr("经度"));
    m_latEdit = new QLineEdit(this);
    m_latEdit->setPlaceholderText(tr("纬度"));
    lonLatLayout->addWidget(new QLabel(tr("经度"), this));
    lonLatLayout->addWidget(m_lonEdit);
    lonLatLayout->addWidget(new QLabel(tr("纬度"), this));
    lonLatLayout->addWidget(m_latEdit);
    auto *lonLatWidget = new QWidget(this);
    lonLatWidget->setLayout(lonLatLayout);
    formLayout->addRow(tr("位置"), lonLatWidget);

    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMinimumHeight(60);
    formLayout->addRow(tr("描述"), m_descriptionEdit);

    m_actionsEdit = new QTextEdit(this);
    m_actionsEdit->setMinimumHeight(60);
    formLayout->addRow(tr("建议/操作"), m_actionsEdit);

    m_assignedToEdit = new QLineEdit(this);
    formLayout->addRow(tr("负责人"), m_assignedToEdit);

    m_planStartEdit = new QDateTimeEdit(this);
    m_planStartEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    m_planStartEdit->setCalendarPopup(true);
    formLayout->addRow(tr("计划开始"), m_planStartEdit);

    m_planEndEdit = new QDateTimeEdit(this);
    m_planEndEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    m_planEndEdit->setCalendarPopup(true);
    formLayout->addRow(tr("计划结束"), m_planEndEdit);

    m_actualStartEdit = new QDateTimeEdit(this);
    m_actualStartEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    m_actualStartEdit->setCalendarPopup(true);
    formLayout->addRow(tr("实际开始"), m_actualStartEdit);

    m_actualEndEdit = new QDateTimeEdit(this);
    m_actualEndEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    m_actualEndEdit->setCalendarPopup(true);
    formLayout->addRow(tr("实际结束"), m_actualEndEdit);

    m_completionSpin = new QSpinBox(this);
    m_completionSpin->setRange(0, 100);
    formLayout->addRow(tr("完成率(%)"), m_completionSpin);

    m_resultEdit = new QTextEdit(this);
    m_resultEdit->setMinimumHeight(60);
    formLayout->addRow(tr("工作结果"), m_resultEdit);

    m_reviewedByEdit = new QLineEdit(this);
    formLayout->addRow(tr("审核人"), m_reviewedByEdit);

    m_reviewResultEdit = new QLineEdit(this);
    formLayout->addRow(tr("审核结果"), m_reviewResultEdit);

    m_reviewCommentsEdit = new QTextEdit(this);
    m_reviewCommentsEdit->setMinimumHeight(40);
    formLayout->addRow(tr("审核意见"), m_reviewCommentsEdit);

    mainLayout->addLayout(formLayout);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton(tr("保存"), this);
    m_cancelBtn = new QPushButton(tr("取消"), this);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &WorkOrderEditDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &WorkOrderEditDialog::onCancel);
}

void WorkOrderEditDialog::populateFields(const WorkOrder &workOrder)
{
    if (workOrder.isValid()) {
        m_orderIdEdit->setText(workOrder.orderId());
    }
    m_titleEdit->setText(workOrder.orderTitle());

    int typeIdx = m_typeCombo->findData(workOrder.orderType());
    if (typeIdx >= 0) m_typeCombo->setCurrentIndex(typeIdx);

    int prIdx = m_priorityCombo->findData(workOrder.priority());
    if (prIdx >= 0) m_priorityCombo->setCurrentIndex(prIdx);

    int stIdx = m_statusCombo->findData(workOrder.status());
    if (stIdx >= 0) m_statusCombo->setCurrentIndex(stIdx);

    int assetIdx = m_assetTypeCombo->findData(workOrder.assetType());
    if (assetIdx >= 0) m_assetTypeCombo->setCurrentIndex(assetIdx);
    m_assetIdEdit->setText(workOrder.assetId());

    if (!workOrder.location().isNull()) {
        m_lonEdit->setText(QString::number(workOrder.location().x(), 'f', 6));
        m_latEdit->setText(QString::number(workOrder.location().y(), 'f', 6));
    }

    m_descriptionEdit->setPlainText(workOrder.description());
    m_actionsEdit->setPlainText(workOrder.requiredActions());
    m_assignedToEdit->setText(workOrder.assignedTo());

    if (workOrder.planStartTime().isValid() && workOrder.planStartTime().date().year() > 2000) {
        m_planStartEdit->setDateTime(workOrder.planStartTime());
    }
    if (workOrder.planEndTime().isValid() && workOrder.planEndTime().date().year() > 2000) {
        m_planEndEdit->setDateTime(workOrder.planEndTime());
    }
    if (workOrder.actualStartTime().isValid() && workOrder.actualStartTime().date().year() > 2000) {
        m_actualStartEdit->setDateTime(workOrder.actualStartTime());
    }
    if (workOrder.actualEndTime().isValid() && workOrder.actualEndTime().date().year() > 2000) {
        m_actualEndEdit->setDateTime(workOrder.actualEndTime());
    }

    m_completionSpin->setValue(workOrder.completionRate());
    m_resultEdit->setPlainText(workOrder.workResult());
    m_reviewedByEdit->setText(workOrder.reviewedBy());
    m_reviewResultEdit->setText(workOrder.reviewResult());
    m_reviewCommentsEdit->setPlainText(workOrder.reviewComments());
}

WorkOrder WorkOrderEditDialog::collectInput() const
{
    WorkOrder wo = m_workOrder;
    wo.setOrderId(m_orderIdEdit->text().trimmed());
    wo.setOrderTitle(m_titleEdit->text().trimmed());
    wo.setOrderType(m_typeCombo->currentData().toString());
    wo.setPriority(m_priorityCombo->currentData().toString());
    wo.setStatus(m_statusCombo->currentData().toString());

    wo.setAssetType(m_assetTypeCombo->currentData().toString());
    wo.setAssetId(m_assetIdEdit->text().trimmed());

    // 位置
    bool okLon = false, okLat = false;
    double lon = m_lonEdit->text().toDouble(&okLon);
    double lat = m_latEdit->text().toDouble(&okLat);
    if (okLon && okLat) {
        wo.setLocation(QPointF(lon, lat));
    }

    wo.setDescription(m_descriptionEdit->toPlainText().trimmed());
    wo.setRequiredActions(m_actionsEdit->toPlainText().trimmed());
    wo.setAssignedTo(m_assignedToEdit->text().trimmed());

    // 只保存用户实际设置的时间（排除默认的2000-01-01）
    QDateTime planStart = m_planStartEdit->dateTime();
    if (planStart.isValid() && planStart.date().year() > 2000) {
        wo.setPlanStartTime(planStart);
    }
    
    QDateTime planEnd = m_planEndEdit->dateTime();
    if (planEnd.isValid() && planEnd.date().year() > 2000) {
        wo.setPlanEndTime(planEnd);
    }
    
    QDateTime actualStart = m_actualStartEdit->dateTime();
    if (actualStart.isValid() && actualStart.date().year() > 2000) {
        wo.setActualStartTime(actualStart);
    }
    
    QDateTime actualEnd = m_actualEndEdit->dateTime();
    if (actualEnd.isValid() && actualEnd.date().year() > 2000) {
        wo.setActualEndTime(actualEnd);
    }

    wo.setCompletionRate(m_completionSpin->value());
    wo.setWorkResult(m_resultEdit->toPlainText().trimmed());
    wo.setReviewedBy(m_reviewedByEdit->text().trimmed());
    wo.setReviewResult(m_reviewResultEdit->text().trimmed());
    wo.setReviewComments(m_reviewCommentsEdit->toPlainText().trimmed());

    return wo;
}

void WorkOrderEditDialog::onSave()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请填写工单标题"));
        return;
    }

    m_workOrder = collectInput();
    accept();
}

void WorkOrderEditDialog::onCancel()
{
    reject();
}

