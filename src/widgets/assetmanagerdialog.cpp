#include "assetmanagerdialog.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include "core/common/logger.h"
#include "ui/pipelineeditdialog.h"
#include "widgets/facilityeditdialog.h"
#include "widgets/assetstatisticsdialog.h"
#include "core/auth/permissionmanager.h"
#include "core/database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QSet>
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>

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
    m_deleteBtn = new QPushButton("删除", this);
    m_statisticsBtn = new QPushButton("统计报表", this);
    m_closeBtn = new QPushButton("关闭", this);
    
    m_viewBtn->setEnabled(false);
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    
    // 设置删除按钮样式（红色警告）
    m_deleteBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 6px 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover:enabled {"
        "    background-color: #d32f2f;"
        "}"
        "QPushButton:pressed:enabled {"
        "    background-color: #b71c1c;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
    );
    
    buttonLayout->addWidget(m_viewBtn);
    buttonLayout->addWidget(m_editBtn);
    buttonLayout->addWidget(m_deleteBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_statisticsBtn);
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
    connect(m_deleteBtn, &QPushButton::clicked, this, &AssetManagerDialog::onDeleteClicked);
    connect(m_statisticsBtn, &QPushButton::clicked, this, &AssetManagerDialog::onStatisticsClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    // 连接复选框状态变化信号
    connect(m_pipelineTable, &QTableWidget::itemChanged, this, &AssetManagerDialog::onCheckBoxStateChanged);
    connect(m_facilityTable, &QTableWidget::itemChanged, this, &AssetManagerDialog::onCheckBoxStateChanged);
    
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
    m_pipelineTable->setColumnCount(9);  // 增加一列用于复选框
    QStringList pipelineHeaders = {
        "", "管线编号", "名称", "类型", "长度(m)", "管径(mm)", 
        "材质", "状态", "健康度"
    };
    m_pipelineTable->setHorizontalHeaderLabels(pipelineHeaders);
    m_pipelineTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pipelineTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pipelineTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pipelineTable->setAlternatingRowColors(true);
    
    QHeaderView *pipelineHeader = m_pipelineTable->horizontalHeader();
    pipelineHeader->setStretchLastSection(true);
    pipelineHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    // 设置复选框列宽度
    m_pipelineTable->setColumnWidth(0, 30);
    
    // 设施表格
    m_facilityTable = new QTableWidget(this);
    m_facilityTable->setColumnCount(8);  // 增加一列用于复选框
    QStringList facilityHeaders = {
        "", "设施编号", "名称", "类型", "规格", "材质", "状态", "健康度"
    };
    m_facilityTable->setHorizontalHeaderLabels(facilityHeaders);
    m_facilityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_facilityTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_facilityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_facilityTable->setAlternatingRowColors(true);
    
    QHeaderView *facilityHeader = m_facilityTable->horizontalHeader();
    facilityHeader->setStretchLastSection(true);
    facilityHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    // 设置复选框列宽度
    m_facilityTable->setColumnWidth(0, 30);
    
    m_tabWidget->addTab(m_pipelineTable, "管线资产");
    m_tabWidget->addTab(m_facilityTable, "设施资产");
}

void AssetManagerDialog::loadPipelines()
{
    qDebug() << "[AssetManager] loadPipelines() called - reloading from database";
    // 创建新的DAO实例，确保不使用缓存
    PipelineDAO dao;
    QVector<Pipeline> pipelines = dao.findAll(1000);
    
    qDebug() << "[AssetManager] Loading pipelines: total=" << pipelines.size() << "from database";
    
    // 暂时断开信号，避免加载时频繁触发
    m_pipelineTable->blockSignals(true);
    int oldRowCount = m_pipelineTable->rowCount();
    m_pipelineTable->setRowCount(0);
    qDebug() << "[AssetManager] Cleared table, old row count:" << oldRowCount;
    
    QString typeFilter = m_typeFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    QString searchText = m_searchEdit->text().toLower();
    
    // 定义设施类型列表（用于判断筛选器选择的是否为设施类型）
    QStringList facilityTypes = {"valve", "manhole", "pump_station", "transformer", "regulator", "junction_box"};
    
    // 如果筛选器选择的是设施类型，则忽略类型筛选（因为这是管线标签页）
    bool isFacilityTypeFilter = facilityTypes.contains(typeFilter);
    if (isFacilityTypeFilter) {
        qDebug() << "[AssetManager] Type filter is facility type, ignoring for pipelines";
        typeFilter = "";  // 清空类型筛选
    }
    
    qDebug() << "[AssetManager] Filters - type:" << typeFilter << "status:" << statusFilter << "search:" << searchText;
    
    int row = 0;
    int filteredByType = 0;
    int filteredByStatus = 0;
    int filteredBySearch = 0;
    
    for (const Pipeline &p : pipelines) {
        // 应用筛选
        if (!typeFilter.isEmpty() && p.pipelineType() != typeFilter) {
            filteredByType++;
            continue;
        }
        if (!statusFilter.isEmpty() && p.status() != statusFilter) {
            filteredByStatus++;
            continue;
        }
        if (!searchText.isEmpty()) {
            QString searchable = p.pipelineId().toLower() + p.pipelineName().toLower();
            if (!searchable.contains(searchText)) {
                filteredBySearch++;
                continue;
            }
        }
        
        m_pipelineTable->insertRow(row);
        
        // 第一列：复选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        m_pipelineTable->setItem(row, 0, checkItem);
        
        // 其他列数据（索引+1）
        m_pipelineTable->setItem(row, 1, new QTableWidgetItem(p.pipelineId()));
        m_pipelineTable->setItem(row, 2, new QTableWidgetItem(p.pipelineName()));
        m_pipelineTable->setItem(row, 3, new QTableWidgetItem(getTypeDisplayName(p.pipelineType(), true)));
        m_pipelineTable->setItem(row, 4, new QTableWidgetItem(QString::number(p.lengthM(), 'f', 1)));
        m_pipelineTable->setItem(row, 5, new QTableWidgetItem(p.diameterMm() > 0 ? QString::number(p.diameterMm()) : ""));
        m_pipelineTable->setItem(row, 6, new QTableWidgetItem(p.material()));
        m_pipelineTable->setItem(row, 7, new QTableWidgetItem(getStatusDisplayName(p.status())));
        m_pipelineTable->setItem(row, 8, new QTableWidgetItem(QString::number(p.healthScore())));
        
        row++;
    }
    
    qDebug() << "[AssetManager] Pipelines loaded: displayed=" << row 
             << "filtered(type)=" << filteredByType 
             << "filtered(status)=" << filteredByStatus 
             << "filtered(search)=" << filteredBySearch;
    
    m_tabWidget->setTabText(0, QString("管线资产 (%1)").arg(row));
    m_pipelineTable->blockSignals(false);
    
    // 强制更新表格视图
    m_pipelineTable->viewport()->update();
    m_pipelineTable->update();
    
    // 更新按钮状态
    onCheckBoxStateChanged();
}

void AssetManagerDialog::loadFacilities()
{
    qDebug() << "[AssetManager] loadFacilities() called - reloading from database";
    // 创建新的DAO实例，确保不使用缓存
    FacilityDAO dao;
    QVector<Facility> facilities = dao.findAll(1000);
    
    qDebug() << "[AssetManager] Loading facilities: total=" << facilities.size() << "from database";
    
    // 暂时断开信号，避免加载时频繁触发
    m_facilityTable->blockSignals(true);
    int oldRowCount = m_facilityTable->rowCount();
    m_facilityTable->setRowCount(0);
    qDebug() << "[AssetManager] Cleared table, old row count:" << oldRowCount;
    
    QString typeFilter = m_typeFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    QString searchText = m_searchEdit->text().toLower();
    
    // 定义管线类型列表（用于判断筛选器选择的是否为管线类型）
    QStringList pipelineTypes = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
    
    // 如果筛选器选择的是管线类型，则忽略类型筛选（因为这是设施标签页）
    bool isPipelineTypeFilter = pipelineTypes.contains(typeFilter);
    if (isPipelineTypeFilter) {
        qDebug() << "[AssetManager] Type filter is pipeline type, ignoring for facilities";
        typeFilter = "";  // 清空类型筛选
    }
    
    qDebug() << "[AssetManager] Filters - type:" << typeFilter << "status:" << statusFilter << "search:" << searchText;
    
    int row = 0;
    int filteredByType = 0;
    int filteredByStatus = 0;
    int filteredBySearch = 0;
    
    for (const Facility &f : facilities) {
        // 应用筛选
        if (!typeFilter.isEmpty() && f.facilityType() != typeFilter) {
            filteredByType++;
            continue;
        }
        if (!statusFilter.isEmpty() && f.status() != statusFilter) {
            filteredByStatus++;
            continue;
        }
        if (!searchText.isEmpty()) {
            QString searchable = f.facilityId().toLower() + f.facilityName().toLower();
            if (!searchable.contains(searchText)) {
                filteredBySearch++;
                continue;
            }
        }
        
        m_facilityTable->insertRow(row);
        
        // 第一列：复选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        m_facilityTable->setItem(row, 0, checkItem);
        
        // 其他列数据（索引+1）
        m_facilityTable->setItem(row, 1, new QTableWidgetItem(f.facilityId()));
        m_facilityTable->setItem(row, 2, new QTableWidgetItem(f.facilityName()));
        m_facilityTable->setItem(row, 3, new QTableWidgetItem(getTypeDisplayName(f.facilityType(), false)));
        m_facilityTable->setItem(row, 4, new QTableWidgetItem(f.spec()));
        m_facilityTable->setItem(row, 5, new QTableWidgetItem(f.material()));
        m_facilityTable->setItem(row, 6, new QTableWidgetItem(getStatusDisplayName(f.status())));
        m_facilityTable->setItem(row, 7, new QTableWidgetItem(QString::number(f.healthScore())));
        
        row++;
    }
    
    qDebug() << "[AssetManager] Facilities loaded: displayed=" << row 
             << "filtered(type)=" << filteredByType 
             << "filtered(status)=" << filteredByStatus 
             << "filtered(search)=" << filteredBySearch;
    
    m_tabWidget->setTabText(1, QString("设施资产 (%1)").arg(row));
    m_facilityTable->blockSignals(false);
    
    // 强制更新表格视图
    m_facilityTable->viewport()->update();
    m_facilityTable->update();
    
    // 更新按钮状态
    onCheckBoxStateChanged();
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
    qDebug() << "[AssetManager] Refresh button clicked, current tab:" << m_currentTabIndex;
    
    // 强制处理事件，确保界面状态更新
    QApplication::processEvents();
    
    if (m_currentTabIndex == 0) {
        // 清空表格并重新加载
        m_pipelineTable->blockSignals(true);
        m_pipelineTable->setRowCount(0);
        m_pipelineTable->blockSignals(false);
        // 强制刷新表格视图
        m_pipelineTable->viewport()->update();
        m_pipelineTable->update();
        QApplication::processEvents();
        // 重新从数据库加载数据
        loadPipelines();
    } else {
        // 清空表格并重新加载
        m_facilityTable->blockSignals(true);
        m_facilityTable->setRowCount(0);
        m_facilityTable->blockSignals(false);
        // 强制刷新表格视图
        m_facilityTable->viewport()->update();
        m_facilityTable->update();
        QApplication::processEvents();
        // 重新从数据库加载数据
        loadFacilities();
    }
    
    // 最终强制刷新界面
    QApplication::processEvents();
    qDebug() << "[AssetManager] Refresh completed";
}

void AssetManagerDialog::onViewClicked()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要查看的资产");
        return;
    }
    
    QString assetId = currentTable->item(row, 1)->text();  // 索引+1，因为第一列是复选框
    
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
    
    // 检查权限
    bool hasPermission = false;
    if (m_currentTabIndex == 0) {
        hasPermission = PermissionManager::canEditPipeline();
    } else {
        hasPermission = PermissionManager::canEditFacility();
    }
    
    if (!hasPermission) {
        QMessageBox::warning(this, "权限不足", "您没有权限编辑该资产。");
        return;
    }
    
    QString assetId = currentTable->item(row, 1)->text();  // 索引+1，因为第一列是复选框
    
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
    int row = currentTable->currentRow();
    
    // 根据权限和选择状态启用/禁用按钮
    m_viewBtn->setEnabled(row >= 0);
    bool canEdit = false;
    if (row >= 0) {
        if (m_currentTabIndex == 0) {
            canEdit = PermissionManager::canEditPipeline();
        } else {
            canEdit = PermissionManager::canEditFacility();
        }
    }
    m_editBtn->setEnabled(canEdit && row >= 0);
    
    // 删除按钮状态由复选框决定，在onCheckBoxStateChanged中更新
    onCheckBoxStateChanged();
}

void AssetManagerDialog::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        onViewClicked();
    }
}

void AssetManagerDialog::onStatisticsClicked()
{
    AssetStatisticsDialog dialog(this);
    dialog.exec();
}

void AssetManagerDialog::onDeleteClicked()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    bool isPipeline = (m_currentTabIndex == 0);
    QString assetType = isPipeline ? "管线" : "设施";
    
    // 1. 获取所有勾选的行
    QList<int> checkedRows;
    QStringList selectedIds;
    QStringList selectedNames;
    
    for (int row = 0; row < currentTable->rowCount(); ++row) {
        QTableWidgetItem *checkItem = currentTable->item(row, 0);
        if (checkItem && checkItem->checkState() == Qt::Checked) {
            checkedRows.append(row);
            QString assetId = currentTable->item(row, 1)->text();
            QString assetName = currentTable->item(row, 2)->text();
            selectedIds.append(assetId);
            selectedNames.append(QString("%1 - %2").arg(assetId).arg(assetName));
        }
    }
    
    // 2. 检查是否有选中的项
    if (checkedRows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先勾选要删除的资产");
        return;
    }
    
    // 3. 检查权限
    bool hasPermission = isPipeline ? PermissionManager::canEditPipeline() : PermissionManager::canEditFacility();
    if (!hasPermission) {
        QMessageBox::warning(this, "权限不足", QString("您没有权限删除%1。").arg(assetType));
        return;
    }
    
    // 4. 确认删除对话框
    int count = checkedRows.count();
    QString confirmMessage;
    QString windowTitle;
    
    if (count == 1) {
        // 单个删除：显示详细信息
        windowTitle = "确认删除";
        confirmMessage = QString("确定要删除以下%1吗？\n\n"
                                "编号：%2\n"
                                "名称：%3\n\n"
                                "⚠️ 此操作不可恢复！")
                            .arg(assetType)
                            .arg(selectedIds[0])
                            .arg(selectedNames[0].split(" - ").last());
    } else {
        // 批量删除：显示数量和列表
        windowTitle = QString("确认批量删除 (%1 条)").arg(count);
        confirmMessage = QString("确定要删除以下 %1 条%2记录吗？\n\n").arg(count).arg(assetType);
        
        // 显示前5条，如果超过5条则显示省略号
        int displayCount = qMin(5, selectedNames.count());
        for (int i = 0; i < displayCount; ++i) {
            confirmMessage += QString("• %1\n").arg(selectedNames[i]);
        }
        if (selectedNames.count() > 5) {
            confirmMessage += QString("... 还有 %1 条\n").arg(selectedNames.count() - 5);
        }
        confirmMessage += "\n⚠️ 此操作不可恢复！";
    }
    
    int ret = QMessageBox::warning(this, windowTitle, confirmMessage,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // 5. 执行删除操作
    int successCount = 0;
    int failCount = 0;
    QStringList failedIds;
    QStringList failedNames;
    QList<int> deletedDbIds;      // 成功删除的数据库ID列表
    QStringList deletedAssetIds;  // 成功删除的资产ID列表（与deletedDbIds一一对应）
    
    if (isPipeline) {
        // 删除管线
        PipelineDAO dao;
        for (int i = 0; i < checkedRows.count(); ++i) {
            QString assetId = selectedIds[i];
            QString assetName = selectedNames[i];
            
            qDebug() << "[AssetManager] Deleting pipeline:" << assetId;
            
            Pipeline p = dao.findByPipelineId(assetId);
            if (!p.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Pipeline not found in database:" << assetId;
                continue;
            }
            
            int dbId = p.id();
            qDebug() << "[AssetManager] Found pipeline in database, id:" << dbId << "pipelineId:" << assetId;
            
            // 执行删除操作 - 直接使用SQL确保删除
            qDebug() << "[AssetManager] Executing DELETE for pipeline id=" << dbId << "pipelineId=" << assetId;
            
            // 先验证记录是否存在
            Pipeline checkPipeline = dao.findById(dbId);
            if (!checkPipeline.isValid()) {
                qWarning() << "[AssetManager] Pipeline not found before deletion, id=" << dbId << "pipelineId=" << assetId;
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                continue;
            }
            
            // 直接使用SQL删除，确保删除操作真正执行
            QString deleteSql = QString("DELETE FROM pipelines WHERE id = :id");
            QVariantMap deleteParams;
            deleteParams[":id"] = dbId;
            
            QSqlQuery deleteQuery = DatabaseManager::instance().executeQuery(deleteSql, deleteParams);
            if (deleteQuery.lastError().isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Pipeline deletion SQL failed:" << assetId 
                           << "Error:" << deleteQuery.lastError().text();
                continue;
            }
            
            // 检查受影响的行数
            int affectedRows = deleteQuery.numRowsAffected();
            qDebug() << "[AssetManager] DELETE executed, affected rows:" << affectedRows;
            
            if (affectedRows <= 0) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Pipeline deletion affected 0 rows, id=" << dbId << "pipelineId=" << assetId;
                continue;
            }
            
            qDebug() << "[AssetManager] DELETE command executed successfully for pipeline id=" << dbId << "affected rows=" << affectedRows;
            
            // 强制处理事件，确保删除操作完成
            QApplication::processEvents();
            
            // 等待一小段时间，确保数据库操作完全提交
            QThread::msleep(100);
            
            // 创建新的DAO实例，确保使用新的数据库连接查询
            PipelineDAO verifyDao;
            // 验证删除是否成功：通过ID查找
            Pipeline verifyById = verifyDao.findById(dbId);
            if (verifyById.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Pipeline deletion verification failed (still exists by id):" << assetId;
                qWarning() << "[AssetManager] Pipeline still in database - id:" << verifyById.id() << "pipelineId:" << verifyById.pipelineId();
                continue;
            }
            
            // 再次验证：通过pipelineId查找（更严格）
            Pipeline verifyByPipelineId = verifyDao.findByPipelineId(assetId);
            if (verifyByPipelineId.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Pipeline deletion verification failed (still exists by pipelineId):" << assetId;
                qWarning() << "[AssetManager] Pipeline still in database - id:" << verifyByPipelineId.id() << "pipelineId:" << verifyByPipelineId.pipelineId();
                continue;
            }
            
            qDebug() << "[AssetManager] Pipeline deletion verified successfully - id=" << dbId << "pipelineId=" << assetId << "no longer exists in database";
            
            // 删除成功，记录信息
            successCount++;
            deletedDbIds.append(dbId);
            deletedAssetIds.append(assetId);
            qDebug() << "[AssetManager] Pipeline deleted successfully: id=" << dbId << "pipelineId=" << assetId;
        }
    } else {
        // 删除设施
        FacilityDAO dao;
        for (int i = 0; i < checkedRows.count(); ++i) {
            QString assetId = selectedIds[i];
            QString assetName = selectedNames[i];
            
            qDebug() << "[AssetManager] Deleting facility:" << assetId;
            
            Facility f = dao.findByFacilityId(assetId);
            if (!f.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Facility not found in database:" << assetId;
                continue;
            }
            
            int dbId = f.id();
            qDebug() << "[AssetManager] Found facility in database, id:" << dbId << "facilityId:" << assetId;
            
            // 执行删除操作 - 直接使用SQL确保删除
            qDebug() << "[AssetManager] Executing DELETE for facility id=" << dbId << "facilityId=" << assetId;
            
            // 先验证记录是否存在
            Facility checkFacility = dao.findById(dbId);
            if (!checkFacility.isValid()) {
                qWarning() << "[AssetManager] Facility not found before deletion, id=" << dbId << "facilityId=" << assetId;
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                continue;
            }
            
            // 直接使用SQL删除，确保删除操作真正执行
            QString deleteSql = QString("DELETE FROM facilities WHERE id = :id");
            QVariantMap deleteParams;
            deleteParams[":id"] = dbId;
            
            QSqlQuery deleteQuery = DatabaseManager::instance().executeQuery(deleteSql, deleteParams);
            if (deleteQuery.lastError().isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Facility deletion SQL failed:" << assetId 
                           << "Error:" << deleteQuery.lastError().text();
                continue;
            }
            
            // 检查受影响的行数
            int affectedRows = deleteQuery.numRowsAffected();
            qDebug() << "[AssetManager] DELETE executed, affected rows:" << affectedRows;
            
            if (affectedRows <= 0) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Facility deletion affected 0 rows, id=" << dbId << "facilityId=" << assetId;
                continue;
            }
            
            qDebug() << "[AssetManager] DELETE command executed successfully for facility id=" << dbId << "affected rows=" << affectedRows;
            
            // 强制处理事件，确保删除操作完成
            QApplication::processEvents();
            
            // 等待一小段时间，确保数据库操作完全提交
            QThread::msleep(100);
            
            // 创建新的DAO实例，确保使用新的数据库连接查询
            FacilityDAO verifyDao;
            // 验证删除是否成功：通过ID查找
            Facility verifyById = verifyDao.findById(dbId);
            if (verifyById.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Facility deletion verification failed (still exists by id):" << assetId;
                qWarning() << "[AssetManager] Facility still in database - id:" << verifyById.id() << "facilityId:" << verifyById.facilityId();
                continue;
            }
            
            // 再次验证：通过facilityId查找（更严格）
            Facility verifyByFacilityId = verifyDao.findByFacilityId(assetId);
            if (verifyByFacilityId.isValid()) {
                failCount++;
                failedIds.append(assetId);
                failedNames.append(assetName);
                qWarning() << "[AssetManager] Facility deletion verification failed (still exists by facilityId):" << assetId;
                qWarning() << "[AssetManager] Facility still in database - id:" << verifyByFacilityId.id() << "facilityId:" << verifyByFacilityId.facilityId();
                continue;
            }
            
            qDebug() << "[AssetManager] Facility deletion verified successfully - id=" << dbId << "facilityId=" << assetId << "no longer exists in database";
            
            // 删除成功，记录信息
            successCount++;
            deletedDbIds.append(dbId);
            deletedAssetIds.append(assetId);
            qDebug() << "[AssetManager] Facility deleted successfully: id=" << dbId << "facilityId=" << assetId;
        }
    }
    
    qDebug() << "[AssetManager] Delete operation completed: success=" << successCount << "fail=" << failCount;
    
    // 6. 发送删除信号（批量发送，通知主窗口刷新地图）
    if (successCount > 0) {
        if (isPipeline) {
            // 批量发送管线删除信号
            for (int i = 0; i < deletedDbIds.count() && i < deletedAssetIds.count(); ++i) {
                emit pipelineDeleted(deletedDbIds[i], deletedAssetIds[i]);
            }
        } else {
            // 批量发送设施删除信号
            for (int i = 0; i < deletedDbIds.count() && i < deletedAssetIds.count(); ++i) {
                emit facilityDeleted(deletedDbIds[i], deletedAssetIds[i]);
            }
        }
    }
    
    // 7. 刷新表格界面
    qDebug() << "[AssetManager] Refreshing table after delete...";
    
    // 强制处理事件，确保所有删除操作都完成
    QApplication::processEvents();
    
    // 等待一小段时间，确保数据库操作完全提交（PostgreSQL默认自动提交）
    QThread::msleep(100);
    
    // 重新加载数据（直接调用load函数，确保从数据库重新查询）
    // 先清空表格，然后重新加载，确保界面同步更新
    if (isPipeline) {
        // 清空表格并重新加载
        m_pipelineTable->blockSignals(true);
        m_pipelineTable->setRowCount(0);
        m_pipelineTable->blockSignals(false);
        // 强制刷新表格视图
        m_pipelineTable->viewport()->update();
        m_pipelineTable->update();
        QApplication::processEvents();
        // 重新从数据库加载数据
        loadPipelines();
    } else {
        // 清空表格并重新加载
        m_facilityTable->blockSignals(true);
        m_facilityTable->setRowCount(0);
        m_facilityTable->blockSignals(false);
        // 强制刷新表格视图
        m_facilityTable->viewport()->update();
        m_facilityTable->update();
        QApplication::processEvents();
        // 重新从数据库加载数据
        loadFacilities();
    }
    
    QApplication::processEvents();
    qDebug() << "[AssetManager] Table refreshed, current row count:" << currentTable->rowCount();
    
    // 验证删除的设备是否还在表格中（用于调试）
    if (successCount > 0 && !deletedAssetIds.isEmpty()) {
        QStringList remainingIds;
        for (int row = 0; row < currentTable->rowCount(); ++row) {
            QString id = currentTable->item(row, 1)->text();
            if (deletedAssetIds.contains(id)) {
                remainingIds.append(id);
            }
        }
        if (!remainingIds.isEmpty()) {
            qWarning() << "[AssetManager] WARNING: Deleted assets still in table after refresh:" << remainingIds;
            qWarning() << "[AssetManager] This indicates the deletion may not have been committed to database";
        } else {
            qDebug() << "[AssetManager] Verified: All deleted assets removed from table";
        }
    }
    
    // 8. 显示操作结果
    QString resultMessage;
    if (failCount == 0) {
        // 全部成功
        if (count == 1) {
            resultMessage = QString("成功删除 %1 条%2记录").arg(successCount).arg(assetType);
        } else {
            resultMessage = QString("成功删除 %1 条%2记录").arg(successCount).arg(assetType);
        }
        QMessageBox::information(this, "删除成功", resultMessage);
    } else if (successCount == 0) {
        // 全部失败
        resultMessage = QString("删除失败：%1 条记录均未能删除\n\n失败的记录：\n%2")
            .arg(failCount)
            .arg(failedNames.join("\n"));
        QMessageBox::critical(this, "删除失败", resultMessage);
    } else {
        // 部分成功
        resultMessage = QString("删除完成：成功 %1 条，失败 %2 条\n\n失败的记录：\n%3")
            .arg(successCount)
            .arg(failCount)
            .arg(failedNames.join("\n"));
        QMessageBox::warning(this, "部分删除失败", resultMessage);
    }
    
    // 9. 最终刷新界面
    QApplication::processEvents();
    if (isPipeline) {
        m_pipelineTable->viewport()->update();
        m_pipelineTable->update();
    } else {
        m_facilityTable->viewport()->update();
        m_facilityTable->update();
    }
}

void AssetManagerDialog::onCheckBoxStateChanged()
{
    QTableWidget *currentTable = (m_currentTabIndex == 0) ? m_pipelineTable : m_facilityTable;
    
    // 统计勾选的数量
    int checkedCount = 0;
    for (int row = 0; row < currentTable->rowCount(); ++row) {
        QTableWidgetItem *checkItem = currentTable->item(row, 0);
        if (checkItem && checkItem->checkState() == Qt::Checked) {
            checkedCount++;
        }
    }
    
    // 检查权限
    bool hasPermission = false;
    if (m_currentTabIndex == 0) {
        hasPermission = PermissionManager::canEditPipeline();
    } else {
        hasPermission = PermissionManager::canEditFacility();
    }
    
    // 根据勾选数量和权限启用/禁用删除按钮
    m_deleteBtn->setEnabled(hasPermission && checkedCount > 0);
    
    // 更新按钮文本，显示选中数量
    if (checkedCount > 0) {
        if (checkedCount == 1) {
            m_deleteBtn->setText("删除");
        } else {
            m_deleteBtn->setText(QString("删除 (%1)").arg(checkedCount));
        }
    } else {
        m_deleteBtn->setText("删除");
    }
}


