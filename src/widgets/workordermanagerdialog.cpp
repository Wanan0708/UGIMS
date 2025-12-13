#include "workordermanagerdialog.h"
#include "dao/workorderdao.h"
#include "workordereditdialog.h"
#include "core/common/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

WorkOrderManagerDialog::WorkOrderManagerDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentSelectedRow(-1)
{
    setWindowTitle("工单管理");
    setMinimumSize(1000, 600);
    setupUI();
    loadWorkOrders();
}

WorkOrderManagerDialog::~WorkOrderManagerDialog()
{
}

void WorkOrderManagerDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 筛选面板
    setupFilterPanel();
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createBtn = new QPushButton("新建工单", this);
    m_editBtn = new QPushButton("编辑", this);
    m_viewBtn = new QPushButton("查看详情", this);
    m_deleteBtn = new QPushButton("删除", this);
    m_closeBtn = new QPushButton("关闭", this);
    
    m_editBtn->setEnabled(false);
    m_viewBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_createBtn);
    buttonLayout->addWidget(m_editBtn);
    buttonLayout->addWidget(m_viewBtn);
    buttonLayout->addWidget(m_deleteBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);
    
    // 工单列表
    setupTable();
    
    m_mainLayout->addWidget(m_filterGroup);
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addWidget(m_tableWidget);
    
    // 连接信号
    connect(m_refreshBtn, &QPushButton::clicked, this, &WorkOrderManagerDialog::onRefreshClicked);
    connect(m_createBtn, &QPushButton::clicked, this, &WorkOrderManagerDialog::onCreateClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &WorkOrderManagerDialog::onEditClicked);
    connect(m_viewBtn, &QPushButton::clicked, this, &WorkOrderManagerDialog::onViewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &WorkOrderManagerDialog::onDeleteClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WorkOrderManagerDialog::onFilterChanged);
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WorkOrderManagerDialog::onFilterChanged);
    connect(m_priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WorkOrderManagerDialog::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &WorkOrderManagerDialog::onFilterChanged);
    
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, 
            this, &WorkOrderManagerDialog::onTableSelectionChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, 
            this, &WorkOrderManagerDialog::onTableDoubleClicked);
}

void WorkOrderManagerDialog::setupFilterPanel()
{
    m_filterGroup = new QGroupBox("筛选条件", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);
    
    m_filterLayout->addWidget(new QLabel("状态:", this));
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItem("全部", "");
    m_statusFilter->addItem("待分配", WorkOrder::STATUS_PENDING);
    m_statusFilter->addItem("已分配", WorkOrder::STATUS_ASSIGNED);
    m_statusFilter->addItem("进行中", WorkOrder::STATUS_IN_PROGRESS);
    m_statusFilter->addItem("已完成", WorkOrder::STATUS_COMPLETED);
    m_statusFilter->addItem("已取消", WorkOrder::STATUS_CANCELLED);
    m_filterLayout->addWidget(m_statusFilter);
    
    m_filterLayout->addWidget(new QLabel("类型:", this));
    m_typeFilter = new QComboBox(this);
    m_typeFilter->addItem("全部", "");
    m_typeFilter->addItem("日常巡检", WorkOrder::TYPE_INSPECTION);
    m_typeFilter->addItem("维修工单", WorkOrder::TYPE_MAINTENANCE);
    m_typeFilter->addItem("应急工单", WorkOrder::TYPE_EMERGENCY);
    m_typeFilter->addItem("改造工单", WorkOrder::TYPE_RENOVATION);
    m_filterLayout->addWidget(m_typeFilter);
    
    m_filterLayout->addWidget(new QLabel("优先级:", this));
    m_priorityFilter = new QComboBox(this);
    m_priorityFilter->addItem("全部", "");
    m_priorityFilter->addItem("低", WorkOrder::PRIORITY_LOW);
    m_priorityFilter->addItem("普通", WorkOrder::PRIORITY_NORMAL);
    m_priorityFilter->addItem("高", WorkOrder::PRIORITY_HIGH);
    m_priorityFilter->addItem("紧急", WorkOrder::PRIORITY_URGENT);
    m_filterLayout->addWidget(m_priorityFilter);
    
    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("工单编号/标题");
    m_filterLayout->addWidget(m_searchEdit);
    
    m_refreshBtn = new QPushButton("刷新", this);
    m_filterLayout->addWidget(m_refreshBtn);
    
    m_filterLayout->addStretch();
}

void WorkOrderManagerDialog::setupTable()
{
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(8);
    QStringList headers = {
        "工单编号", "标题", "类型", "优先级", "状态", 
        "负责人", "创建时间", "计划完成时间"
    };
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    
    // 设置列宽
    QHeaderView *header = m_tableWidget->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void WorkOrderManagerDialog::loadWorkOrders()
{
    WorkOrderDAO dao;
    m_workOrders = dao.findAll(1000);
    refreshTable();
}

void WorkOrderManagerDialog::refreshTable()
{
    m_tableWidget->setRowCount(0);
    
    QString statusFilter = m_statusFilter->currentData().toString();
    QString typeFilter = m_typeFilter->currentData().toString();
    QString priorityFilter = m_priorityFilter->currentData().toString();
    QString searchText = m_searchEdit->text().toLower();
    
    int row = 0;
    for (const WorkOrder &wo : m_workOrders) {
        // 应用筛选
        if (!statusFilter.isEmpty() && wo.status() != statusFilter) continue;
        if (!typeFilter.isEmpty() && wo.orderType() != typeFilter) continue;
        if (!priorityFilter.isEmpty() && wo.priority() != priorityFilter) continue;
        if (!searchText.isEmpty()) {
            QString searchable = wo.orderId().toLower() + wo.orderTitle().toLower();
            if (!searchable.contains(searchText)) continue;
        }
        
        m_tableWidget->insertRow(row);
        
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(wo.orderId()));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(wo.orderTitle()));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(getTypeDisplayName(wo.orderType())));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(getPriorityDisplayName(wo.priority())));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(getStatusDisplayName(wo.status())));
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(wo.assignedTo().isEmpty() ? "未分配" : wo.assignedTo()));
        
        QString createTime = wo.createdAt().isValid() ? 
                             wo.createdAt().toString("yyyy-MM-dd hh:mm") : "";
        m_tableWidget->setItem(row, 6, new QTableWidgetItem(createTime));
        
        QString planEndTime = wo.planEndTime().isValid() ? 
                              wo.planEndTime().toString("yyyy-MM-dd hh:mm") : "";
        m_tableWidget->setItem(row, 7, new QTableWidgetItem(planEndTime));
        
        row++;
    }
    
    // 更新状态栏信息
    setWindowTitle(QString("工单管理 - 共 %1 条记录").arg(row));
}

WorkOrder WorkOrderManagerDialog::getSelectedWorkOrder()
{
    int row = m_tableWidget->currentRow();
    if (row < 0 || row >= m_tableWidget->rowCount()) {
        return WorkOrder();
    }
    
    QString orderId = m_tableWidget->item(row, 0)->text();
    WorkOrderDAO dao;
    return dao.findByOrderId(orderId);
}

QString WorkOrderManagerDialog::getStatusDisplayName(const QString &status)
{
    if (status == WorkOrder::STATUS_PENDING) return "待分配";
    if (status == WorkOrder::STATUS_ASSIGNED) return "已分配";
    if (status == WorkOrder::STATUS_IN_PROGRESS) return "进行中";
    if (status == WorkOrder::STATUS_COMPLETED) return "已完成";
    if (status == WorkOrder::STATUS_CANCELLED) return "已取消";
    return status;
}

QString WorkOrderManagerDialog::getTypeDisplayName(const QString &type)
{
    if (type == WorkOrder::TYPE_INSPECTION) return "日常巡检";
    if (type == WorkOrder::TYPE_MAINTENANCE) return "维修工单";
    if (type == WorkOrder::TYPE_EMERGENCY) return "应急工单";
    if (type == WorkOrder::TYPE_RENOVATION) return "改造工单";
    return type;
}

QString WorkOrderManagerDialog::getPriorityDisplayName(const QString &priority)
{
    if (priority == WorkOrder::PRIORITY_LOW) return "低";
    if (priority == WorkOrder::PRIORITY_NORMAL) return "普通";
    if (priority == WorkOrder::PRIORITY_HIGH) return "高";
    if (priority == WorkOrder::PRIORITY_URGENT) return "紧急";
    return priority;
}

void WorkOrderManagerDialog::onRefreshClicked()
{
    loadWorkOrders();
}

void WorkOrderManagerDialog::onCreateClicked()
{
    WorkOrderDAO dao;
    WorkOrder newWo;
    newWo.setOrderId(dao.generateOrderId());
    newWo.setPriority(WorkOrder::PRIORITY_NORMAL);
    newWo.setStatus(WorkOrder::STATUS_PENDING);
    newWo.setOrderType(WorkOrder::TYPE_INSPECTION);

    WorkOrderEditDialog dlg(this, newWo);
    if (dlg.exec() == QDialog::Accepted) {
        WorkOrder wo = dlg.resultWorkOrder();
        if (wo.orderId().isEmpty()) {
            wo.setOrderId(dao.generateOrderId());
        }
        // 验证必需字段
        if (wo.orderTitle().isEmpty()) {
            QMessageBox::warning(this, "错误", "工单标题不能为空");
            return;
        }
        if (wo.orderId().isEmpty()) {
            QMessageBox::warning(this, "错误", "工单编号不能为空");
            return;
        }
        
        if (!dao.insert(wo)) {
            QString errorMsg = QString("保存工单失败\n\n工单编号: %1\n工单标题: %2")
                              .arg(wo.orderId())
                              .arg(wo.orderTitle());
            QMessageBox::warning(this, "错误", errorMsg);
        } else {
            QMessageBox::information(this, "成功", QString("工单 %1 已保存").arg(wo.orderId()));
            loadWorkOrders();
        }
    }
}

void WorkOrderManagerDialog::onEditClicked()
{
    WorkOrder wo = getSelectedWorkOrder();
    if (!wo.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的工单");
        return;
    }

    WorkOrderEditDialog dlg(this, wo);
    if (dlg.exec() == QDialog::Accepted) {
        WorkOrder updated = dlg.resultWorkOrder();
        updated.setId(wo.id()); // 确保ID用于更新
        WorkOrderDAO dao;
        if (!dao.update(updated)) {
            QString errorMsg = QString("更新工单失败\n\n工单编号: %1").arg(updated.orderId());
            QMessageBox::warning(this, "错误", errorMsg);
        } else {
            QMessageBox::information(this, "成功", QString("工单 %1 已更新").arg(updated.orderId()));
            loadWorkOrders();
        }
    }
}

void WorkOrderManagerDialog::onViewClicked()
{
    WorkOrder wo = getSelectedWorkOrder();
    if (!wo.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要查看的工单");
        return;
    }
    
    // 显示工单详情
    QString details = QString(
        "工单详情\n\n"
        "工单编号: %1\n"
        "标题: %2\n"
        "类型: %3\n"
        "优先级: %4\n"
        "状态: %5\n"
        "负责人: %6\n"
        "描述: %7\n"
        "创建时间: %8\n"
        "计划开始: %9\n"
        "计划完成: %10\n"
        "实际开始: %11\n"
        "实际完成: %12\n"
        "完成率: %13%\n"
        "工作结果: %14\n"
    )
    .arg(wo.orderId())
    .arg(wo.orderTitle())
    .arg(getTypeDisplayName(wo.orderType()))
    .arg(getPriorityDisplayName(wo.priority()))
    .arg(getStatusDisplayName(wo.status()))
    .arg(wo.assignedTo().isEmpty() ? "未分配" : wo.assignedTo())
    .arg(wo.description())
    .arg(wo.createdAt().isValid() ? wo.createdAt().toString("yyyy-MM-dd hh:mm:ss") : "")
    .arg(wo.planStartTime().isValid() ? wo.planStartTime().toString("yyyy-MM-dd hh:mm") : "")
    .arg(wo.planEndTime().isValid() ? wo.planEndTime().toString("yyyy-MM-dd hh:mm") : "")
    .arg(wo.actualStartTime().isValid() ? wo.actualStartTime().toString("yyyy-MM-dd hh:mm") : "")
    .arg(wo.actualEndTime().isValid() ? wo.actualEndTime().toString("yyyy-MM-dd hh:mm") : "")
    .arg(wo.completionRate())
    .arg(wo.workResult());
    
    QMessageBox::information(this, "工单详情", details);
}

void WorkOrderManagerDialog::onDeleteClicked()
{
    WorkOrder wo = getSelectedWorkOrder();
    if (!wo.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的工单");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认删除", 
                                     QString("确定要删除工单 %1 吗？").arg(wo.orderId()),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        WorkOrderDAO dao;
        if (!dao.deleteByOrderId(wo.orderId())) {
            QMessageBox::warning(this, "错误", "删除工单失败");
        } else {
            loadWorkOrders();
        }
    }
}

void WorkOrderManagerDialog::onFilterChanged()
{
    refreshTable();
}

void WorkOrderManagerDialog::onTableSelectionChanged()
{
    bool hasSelection = m_tableWidget->currentRow() >= 0;
    m_editBtn->setEnabled(hasSelection);
    m_viewBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);
}

void WorkOrderManagerDialog::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        onViewClicked();
    }
}

