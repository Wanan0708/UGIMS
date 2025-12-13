#include "assetmanagerdialog.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include "core/common/logger.h"
#include "ui/pipelineeditdialog.h"
#include "widgets/facilityeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

AssetManagerDialog::AssetManagerDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentTabIndex(0)
{
    setWindowTitle("资产管理");
    setMinimumSize(1200, 700);
    setupUI();
    loadPipelines();
    loadFacilities();
}

AssetManagerDialog::~AssetManagerDialog()
{
}

void AssetManagerDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 筛选面板
    setupFilterPanel();
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_viewBtn = new QPushButton("查看详情", this);
    m_editBtn = new QPushButton("编辑", this);
    m_closeBtn = new QPushButton("关闭", this);
    
    m_viewBtn->setEnabled(false);
    m_editBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_viewBtn);
    buttonLayout->addWidget(m_editBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);
    
    // 标签页和表格
    setupTables();
    
    m_mainLayout->addWidget(m_filterGroup);
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addWidget(m_tabWidget);
    
    // 连接信号
    connect(m_refreshBtn, &QPushButton::clicked, this, &AssetManagerDialog::onRefreshClicked);
    connect(m_viewBtn, &QPushButton::clicked, this, &AssetManagerDialog::onViewClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &AssetManagerDialog::onEditClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AssetManagerDialog::onFilterChanged);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AssetManagerDialog::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AssetManagerDialog::onFilterChanged);
    
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &AssetManagerDialog::onTabChanged);
    
    connect(m_pipelineTable, &QTableWidget::itemSelectionChanged, 
            this, &AssetManagerDialog::onTableSelectionChanged);
    connect(m_pipelineTable, &QTableWidget::cellDoubleClicked, 
            this, &AssetManagerDialog::onTableDoubleClicked);
    
    connect(m_facilityTable, &QTableWidget::itemSelectionChanged, 
            this, &AssetManagerDialog::onTableSelectionChanged);
    connect(m_facilityTable, &QTableWidget::cellDoubleClicked, 
            this, &AssetManagerDialog::onTableDoubleClicked);
}

void AssetManagerDialog::setupFilterPanel()
{
    m_filterGroup = new QGroupBox("筛选条件", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);
    
    m_filterLayout->addWidget(new QLabel("类型:", this));
    m_typeFilter = new QComboBox(this);
    m_typeFilter->addItem("全部", "");
    // 管线类型
    m_typeFilter->addItem("给水管线", "water_supply");
    m_typeFilter->addItem("排水管线", "sewage");
    m_typeFilter->addItem("燃气管线", "gas");
    m_typeFilter->addItem("电力电缆", "electric");
    m_typeFilter->addItem("通信线缆", "telecom");
    m_typeFilter->addItem("供热管线", "heat");
    // 设施类型
    m_typeFilter->addItem("阀门", "valve");
    m_typeFilter->addItem("井盖", "manhole");
    m_typeFilter->addItem("泵站", "pump_station");
    m_filterLayout->addWidget(m_typeFilter);
    
    m_filterLayout->addWidget(new QLabel("状态:", this));
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItem("全部", "");
    m_statusFilter->addItem("运行中", "active");
    m_statusFilter->addItem("停用", "inactive");
    m_statusFilter->addItem("维修中", "repairing");
    m_statusFilter->addItem("废弃", "abandoned");
    m_filterLayout->addWidget(m_statusFilter);
    
    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("资产编号/名称");
    m_filterLayout->addWidget(m_searchEdit);
    
    m_refreshBtn = new QPushButton("刷新", this);
    m_filterLayout->addWidget(m_refreshBtn);
    
    m_filterLayout->addStretch();
}

void AssetManagerDialog::setupTables()
{
    m_tabWidget = new QTabWidget(this);
    
    // 管线表格
    m_pipelineTable = new QTableWidget(this);
    m_pipelineTable->setColumnCount(8);
    QStringList pipelineHeaders = {
        "管线编号", "名称", "类型", "长度(m)", "管径(mm)", 
        "材质", "状态", "健康度"
    };
    m_pipelineTable->setHorizontalHeaderLabels(pipelineHeaders);
    m_pipelineTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pipelineTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pipelineTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pipelineTable->setAlternatingRowColors(true);
    
    QHeaderView *pipelineHeader = m_pipelineTable->horizontalHeader();
    pipelineHeader->setStretchLastSection(true);
    pipelineHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    // 设施表格
    m_facilityTable = new QTableWidget(this);
    m_facilityTable->setColumnCount(7);
    QStringList facilityHeaders = {
        "设施编号", "名称", "类型", "规格", "材质", "状态", "健康度"
    };
    m_facilityTable->setHorizontalHeaderLabels(facilityHeaders);
    m_facilityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_facilityTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_facilityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_facilityTable->setAlternatingRowColors(true);
    
    QHeaderView *facilityHeader = m_facilityTable->horizontalHeader();
    facilityHeader->setStretchLastSection(true);
    facilityHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    m_tabWidget->addTab(m_pipelineTable, "管线资产");
    m_tabWidget->addTab(m_facilityTable, "设施资产");
}

void AssetManagerDialog::loadPipelines()
{
    PipelineDAO dao;
    QVector<Pipeline> pipelines = dao.findAll(1000);
    
    m_pipelineTable->setRowCount(0);
    
    QString typeFilter = m_typeFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    QString searchText = m_searchEdit->text().toLower();
    
    int row = 0;
    for (const Pipeline &p : pipelines) {
        // 应用筛选
        if (!typeFilter.isEmpty() && p.pipelineType() != typeFilter) continue;
        if (!statusFilter.isEmpty() && p.status() != statusFilter) continue;
        if (!searchText.isEmpty()) {
            QString searchable = p.pipelineId().toLower() + p.pipelineName().toLower();
            if (!searchable.contains(searchText)) continue;
        }
        
        m_pipelineTable->insertRow(row);
        
        m_pipelineTable->setItem(row, 0, new QTableWidgetItem(p.pipelineId()));
        m_pipelineTable->setItem(row, 1, new QTableWidgetItem(p.pipelineName()));
        m_pipelineTable->setItem(row, 2, new QTableWidgetItem(getTypeDisplayName(p.pipelineType(), true)));
        m_pipelineTable->setItem(row, 3, new QTableWidgetItem(QString::number(p.lengthM(), 'f', 1)));
        m_pipelineTable->setItem(row, 4, new QTableWidgetItem(p.diameterMm() > 0 ? QString::number(p.diameterMm()) : ""));
        m_pipelineTable->setItem(row, 5, new QTableWidgetItem(p.material()));
        m_pipelineTable->setItem(row, 6, new QTableWidgetItem(getStatusDisplayName(p.status())));
        m_pipelineTable->setItem(row, 7, new QTableWidgetItem(QString::number(p.healthScore())));
        
        row++;
    }
    
    m_tabWidget->setTabText(0, QString("管线资产 (%1)").arg(row));
}

void AssetManagerDialog::loadFacilities()
{
    FacilityDAO dao;
    QVector<Facility> facilities = dao.findAll(1000);
    
    m_facilityTable->setRowCount(0);
    
    QString typeFilter = m_typeFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    QString searchText = m_searchEdit->text().toLower();
    
    int row = 0;
    for (const Facility &f : facilities) {
        // 应用筛选
        if (!typeFilter.isEmpty() && f.facilityType() != typeFilter) continue;
        if (!statusFilter.isEmpty() && f.status() != statusFilter) continue;
        if (!searchText.isEmpty()) {
            QString searchable = f.facilityId().toLower() + f.facilityName().toLower();
            if (!searchable.contains(searchText)) continue;
        }
        
        m_facilityTable->insertRow(row);
        
        m_facilityTable->setItem(row, 0, new QTableWidgetItem(f.facilityId()));
        m_facilityTable->setItem(row, 1, new QTableWidgetItem(f.facilityName()));
        m_facilityTable->setItem(row, 2, new QTableWidgetItem(getTypeDisplayName(f.facilityType(), false)));
        m_facilityTable->setItem(row, 3, new QTableWidgetItem(f.spec()));
        m_facilityTable->setItem(row, 4, new QTableWidgetItem(f.material()));
        m_facilityTable->setItem(row, 5, new QTableWidgetItem(getStatusDisplayName(f.status())));
        m_facilityTable->setItem(row, 6, new QTableWidgetItem(QString::number(f.healthScore())));
        
        row++;
    }
    
    m_tabWidget->setTabText(1, QString("设施资产 (%1)").arg(row));
}

void AssetManagerDialog::refreshPipelineTable()
{
    loadPipelines();
}

void AssetManagerDialog::refreshFacilityTable()
{
    loadFacilities();
}

QString AssetManagerDialog::getStatusDisplayName(const QString &status)
{
    if (status == "active") return "运行中";
    if (status == "inactive") return "停用";
    if (status == "repairing") return "维修中";
    if (status == "abandoned") return "废弃";
    return status;
}

QString AssetManagerDialog::getTypeDisplayName(const QString &type, bool isPipeline)
{
    if (isPipeline) {
        if (type == "water_supply") return "给水管线";
        if (type == "sewage") return "排水管线";
        if (type == "gas") return "燃气管线";
        if (type == "electric") return "电力电缆";
        if (type == "telecom") return "通信线缆";
        if (type == "heat") return "供热管线";
    } else {
        if (type == "valve") return "阀门";
        if (type == "manhole") return "井盖";
        if (type == "pump_station") return "泵站";
        if (type == "pressure_station") return "调压站";
        if (type == "transformer") return "变压器";
        if (type == "junction_box") return "接线盒";
    }
    return type;
}

void AssetManagerDialog::onRefreshClicked()
{
    if (m_currentTabIndex == 0) {
        loadPipelines();
    } else {
        loadFacilities();
    }
}

void AssetManagerDialog::onViewClicked()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要查看的资产");
        return;
    }
    
    QString assetId = currentTable->item(row, 0)->text();
    
    if (m_currentTabIndex == 0) {
        // 查看管线详情
        PipelineDAO dao;
        Pipeline p = dao.findByPipelineId(assetId);
        if (!p.isValid()) {
            QMessageBox::warning(this, "错误", "未找到该管线");
            return;
        }
        
        QString details = QString(
            "管线详情\n\n"
            "管线编号: %1\n"
            "名称: %2\n"
            "类型: %3\n"
            "长度: %4 m\n"
            "管径: %5 mm\n"
            "材质: %6\n"
            "状态: %7\n"
            "健康度: %8\n"
            "建设日期: %9\n"
            "建设单位: %10\n"
        )
        .arg(p.pipelineId())
        .arg(p.pipelineName())
        .arg(getTypeDisplayName(p.pipelineType(), true))
        .arg(p.lengthM(), 0, 'f', 1)
        .arg(p.diameterMm())
        .arg(p.material())
        .arg(getStatusDisplayName(p.status()))
        .arg(p.healthScore())
        .arg(p.buildDate().isValid() ? p.buildDate().toString("yyyy-MM-dd") : "")
        .arg(p.builder());
        
        QMessageBox::information(this, "管线详情", details);
    } else {
        // 查看设施详情
        FacilityDAO dao;
        Facility f = dao.findByFacilityId(assetId);
        if (!f.isValid()) {
            QMessageBox::warning(this, "错误", "未找到该设施");
            return;
        }
        
        QString details = QString(
            "设施详情\n\n"
            "设施编号: %1\n"
            "名称: %2\n"
            "类型: %3\n"
            "规格: %4\n"
            "材质: %5\n"
            "状态: %6\n"
            "健康度: %7\n"
            "建设日期: %8\n"
            "建设单位: %9\n"
        )
        .arg(f.facilityId())
        .arg(f.facilityName())
        .arg(getTypeDisplayName(f.facilityType(), false))
        .arg(f.spec())
        .arg(f.material())
        .arg(getStatusDisplayName(f.status()))
        .arg(f.healthScore())
        .arg(f.buildDate().isValid() ? f.buildDate().toString("yyyy-MM-dd") : "")
        .arg(f.builder());
        
        QMessageBox::information(this, "设施详情", details);
    }
}

void AssetManagerDialog::onEditClicked()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的资产");
        return;
    }
    
    QString assetId = currentTable->item(row, 0)->text();
    
    if (m_currentTabIndex == 0) {
        // 编辑管线
        PipelineDAO dao;
        Pipeline p = dao.findByPipelineId(assetId);
        if (!p.isValid()) {
            QMessageBox::warning(this, "错误", "未找到该管线");
            return;
        }
        
        PipelineEditDialog *dlg = new PipelineEditDialog(this);
        dlg->loadPipeline(p);
        if (dlg->exec() == QDialog::Accepted) {
            Pipeline updated = dlg->getPipeline();
            updated.setId(p.id()); // 确保ID用于更新
            
            // 不立即保存，而是发出信号通知主窗口
            emit pipelineModified(updated, p.id());
            
            QMessageBox::information(this, "提示", 
                QString("管线 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                .arg(updated.pipelineId()));
            refreshPipelineTable();
        }
        delete dlg;
    } else {
        // 编辑设施
        FacilityDAO dao;
        Facility f = dao.findByFacilityId(assetId);
        if (!f.isValid()) {
            QMessageBox::warning(this, "错误", "未找到该设施");
            return;
        }
        
        FacilityEditDialog *dlg = new FacilityEditDialog(this, f);
        if (dlg->exec() == QDialog::Accepted) {
            Facility updated = dlg->resultFacility();
            updated.setId(f.id()); // 确保ID用于更新
            
            // 不立即保存，而是发出信号通知主窗口
            emit facilityModified(updated, f.id());
            
            QMessageBox::information(this, "提示", 
                QString("设施 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                .arg(updated.facilityId()));
            refreshFacilityTable();
        }
        delete dlg;
    }
}

void AssetManagerDialog::onFilterChanged()
{
    if (m_currentTabIndex == 0) {
        refreshPipelineTable();
    } else {
        refreshFacilityTable();
    }
}

void AssetManagerDialog::onTabChanged(int index)
{
    m_currentTabIndex = index;
    onFilterChanged();
}

void AssetManagerDialog::onTableSelectionChanged()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    bool hasSelection = currentTable->currentRow() >= 0;
    m_viewBtn->setEnabled(hasSelection);
    m_editBtn->setEnabled(hasSelection);
}

void AssetManagerDialog::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        onViewClicked();
    }
}

