#include "assetstatisticsdialog.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include "core/common/logger.h"
#include "core/auth/permissionmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDate>
#include <QMap>
#include <QList>
#include <QTextStream>
#include <QFile>
#include <QPrinter>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#include <QPageSize>
#include <QPageLayout>
#endif
#include <algorithm>

AssetStatisticsDialog::AssetStatisticsDialog(QWidget *parent)
    : QDialog(parent)
    , m_isPipelineStatistics(true)
    , m_currentDimension("type")
{
    setWindowTitle("资产统计报表");
    setMinimumSize(900, 600);
    setupUI();
    refreshStatistics();
}

AssetStatisticsDialog::~AssetStatisticsDialog()
{
}

void AssetStatisticsDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 统计类型和维度选择
    setupStatisticsPanel();
    
    // 统计结果表格
    m_statisticsTable = new QTableWidget(this);
    m_statisticsTable->setColumnCount(3);
    QStringList headers = {"分类", "数量", "占比"};
    m_statisticsTable->setHorizontalHeaderLabels(headers);
    m_statisticsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statisticsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_statisticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statisticsTable->setAlternatingRowColors(true);
    
    QHeaderView *header = m_statisticsTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    // 统计摘要
    QGroupBox *summaryGroup = new QGroupBox("统计摘要", this);
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryGroup);
    m_totalLabel = new QLabel("总计: 0", this);
    m_summaryLabel = new QLabel("", this);
    m_totalLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    summaryLayout->addWidget(m_totalLabel);
    summaryLayout->addWidget(m_summaryLabel);
    
    // 导出面板
    setupExportPanel();
    
    m_mainLayout->addWidget(m_statisticsTypeCombo->parentWidget());
    m_mainLayout->addWidget(m_statisticsTable);
    m_mainLayout->addWidget(summaryGroup);
    m_mainLayout->addLayout(m_exportLayout);
    
    // 连接信号
    connect(m_statisticsTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AssetStatisticsDialog::onStatisticsTypeChanged);
    connect(m_dimensionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AssetStatisticsDialog::onDimensionChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AssetStatisticsDialog::onRefreshClicked);
    connect(m_exportExcelBtn, &QPushButton::clicked, this, &AssetStatisticsDialog::onExportExcelClicked);
    connect(m_exportPdfBtn, &QPushButton::clicked, this, &AssetStatisticsDialog::onExportPdfClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void AssetStatisticsDialog::setupStatisticsPanel()
{
    QGroupBox *statisticsGroup = new QGroupBox("统计设置", this);
    QHBoxLayout *statisticsLayout = new QHBoxLayout(statisticsGroup);
    
    statisticsLayout->addWidget(new QLabel("统计对象:", this));
    m_statisticsTypeCombo = new QComboBox(this);
    m_statisticsTypeCombo->addItem("管线资产", "pipeline");
    m_statisticsTypeCombo->addItem("设施资产", "facility");
    statisticsLayout->addWidget(m_statisticsTypeCombo);
    
    statisticsLayout->addWidget(new QLabel("统计维度:", this));
    m_dimensionCombo = new QComboBox(this);
    m_dimensionCombo->addItem("按类型", "type");
    m_dimensionCombo->addItem("按状态", "status");
    m_dimensionCombo->addItem("按材质", "material");
    m_dimensionCombo->addItem("按建设年代", "build_year");
    m_dimensionCombo->addItem("按健康度等级", "health_level");
    statisticsLayout->addWidget(m_dimensionCombo);
    
    statisticsLayout->addStretch();
}

void AssetStatisticsDialog::setupExportPanel()
{
    m_exportLayout = new QHBoxLayout;
    
    m_refreshBtn = new QPushButton("刷新", this);
    m_exportExcelBtn = new QPushButton("导出Excel", this);
    m_exportPdfBtn = new QPushButton("导出PDF", this);
    m_closeBtn = new QPushButton("关闭", this);
    
    m_exportLayout->addWidget(m_refreshBtn);
    m_exportLayout->addStretch();
    m_exportLayout->addWidget(m_exportExcelBtn);
    m_exportLayout->addWidget(m_exportPdfBtn);
    m_exportLayout->addWidget(m_closeBtn);
}

void AssetStatisticsDialog::onStatisticsTypeChanged(int index)
{
    Q_UNUSED(index);
    m_isPipelineStatistics = (m_statisticsTypeCombo->currentData().toString() == "pipeline");
    
    // 更新维度选项（管线特有：按长度、按管径）
    m_dimensionCombo->clear();
    m_dimensionCombo->addItem("按类型", "type");
    m_dimensionCombo->addItem("按状态", "status");
    m_dimensionCombo->addItem("按材质", "material");
    m_dimensionCombo->addItem("按建设年代", "build_year");
    m_dimensionCombo->addItem("按健康度等级", "health_level");
    
    if (m_isPipelineStatistics) {
        m_dimensionCombo->addItem("按长度区间", "length");
        m_dimensionCombo->addItem("按管径区间", "diameter");
    }
    
    refreshStatistics();
}

void AssetStatisticsDialog::onDimensionChanged(int index)
{
    Q_UNUSED(index);
    m_currentDimension = m_dimensionCombo->currentData().toString();
    refreshStatistics();
}

void AssetStatisticsDialog::onRefreshClicked()
{
    refreshStatistics();
}

void AssetStatisticsDialog::refreshStatistics()
{
    if (m_currentDimension.isEmpty()) {
        m_currentDimension = m_dimensionCombo->currentData().toString();
    }
    
    if (m_isPipelineStatistics) {
        if (m_currentDimension == "type") {
            calculateByType();
        } else if (m_currentDimension == "status") {
            calculateByStatus();
        } else if (m_currentDimension == "material") {
            calculateByMaterial();
        } else if (m_currentDimension == "build_year") {
            calculateByBuildYear();
        } else if (m_currentDimension == "health_level") {
            calculateByHealthLevel();
        } else if (m_currentDimension == "length") {
            calculateByLength();
        } else if (m_currentDimension == "diameter") {
            calculateByDiameter();
        }
    } else {
        // 设施统计
        if (m_currentDimension == "type") {
            calculateByType();
        } else if (m_currentDimension == "status") {
            calculateByStatus();
        } else if (m_currentDimension == "material") {
            calculateByMaterial();
        } else if (m_currentDimension == "build_year") {
            calculateByBuildYear();
        } else if (m_currentDimension == "health_level") {
            calculateByHealthLevel();
        }
    }
}

void AssetStatisticsDialog::calculateByType()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QStringList types = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
        QStringList typeNames = {"给水管线", "排水管线", "燃气管线", "电力电缆", "通信线缆", "供热管线"};
        
        for (int i = 0; i < types.size(); i++) {
            QVector<Pipeline> pipelines = dao.findByType(types[i], 10000);
            int count = pipelines.size();
            statistics[typeNames[i]] = count;
            total += count;
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        QMap<QString, int> typeCount;
        for (const Facility &facility : facilities) {
            QString type = facility.facilityType();
            typeCount[type]++;
            total++;
        }
        
        for (auto it = typeCount.constBegin(); it != typeCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "分类" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1").arg(total));
    m_summaryLabel->setText(QString("共 %1 个分类").arg(statistics.size()));
}

void AssetStatisticsDialog::calculateByStatus()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QVector<Pipeline> pipelines = dao.findAll(10000);
        
        QMap<QString, int> statusCount;
        for (const Pipeline &pipeline : pipelines) {
            QString status = pipeline.status().isEmpty() ? "未知" : pipeline.status();
            statusCount[status]++;
            total++;
        }
        
        for (auto it = statusCount.constBegin(); it != statusCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        QMap<QString, int> statusCount;
        for (const Facility &facility : facilities) {
            QString status = facility.status().isEmpty() ? "未知" : facility.status();
            statusCount[status]++;
            total++;
        }
        
        for (auto it = statusCount.constBegin(); it != statusCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "状态" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1").arg(total));
    m_summaryLabel->setText(QString("共 %1 种状态").arg(statistics.size()));
}

void AssetStatisticsDialog::calculateByMaterial()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QVector<Pipeline> pipelines = dao.findAll(10000);
        
        QMap<QString, int> materialCount;
        for (const Pipeline &pipeline : pipelines) {
            QString material = pipeline.material().isEmpty() ? "未知" : pipeline.material();
            materialCount[material]++;
            total++;
        }
        
        for (auto it = materialCount.constBegin(); it != materialCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        QMap<QString, int> materialCount;
        for (const Facility &facility : facilities) {
            QString material = facility.material().isEmpty() ? "未知" : facility.material();
            materialCount[material]++;
            total++;
        }
        
        for (auto it = materialCount.constBegin(); it != materialCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "材质" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1").arg(total));
    m_summaryLabel->setText(QString("共 %1 种材质").arg(statistics.size()));
}

void AssetStatisticsDialog::calculateByBuildYear()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QVector<Pipeline> pipelines = dao.findAll(10000);
        
        QMap<QString, int> yearCount;
        for (const Pipeline &pipeline : pipelines) {
            QDate buildDate = pipeline.buildDate();
            QString yearRange = buildDate.isValid() ? getBuildYearRange(buildDate.year()) : "未知";
            yearCount[yearRange]++;
            total++;
        }
        
        for (auto it = yearCount.constBegin(); it != yearCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        QMap<QString, int> yearCount;
        for (const Facility &facility : facilities) {
            QDate buildDate = facility.buildDate();
            QString yearRange = buildDate.isValid() ? getBuildYearRange(buildDate.year()) : "未知";
            yearCount[yearRange]++;
            total++;
        }
        
        for (auto it = yearCount.constBegin(); it != yearCount.constEnd(); ++it) {
            statistics[it.key()] = it.value();
        }
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "建设年代" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1").arg(total));
    m_summaryLabel->setText(QString("共 %1 个年代区间").arg(statistics.size()));
}

void AssetStatisticsDialog::calculateByHealthLevel()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    // 初始化健康度等级
    QStringList levels = {"优秀(90-100)", "良好(80-89)", "一般(60-79)", "较差(40-59)", "危险(0-39)"};
    for (const QString &level : levels) {
        statistics[level] = 0;
    }
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QVector<Pipeline> pipelines = dao.findAll(10000);
        
        for (const Pipeline &pipeline : pipelines) {
            int score = pipeline.healthScore();
            QString level;
            if (score >= 90) level = "优秀(90-100)";
            else if (score >= 80) level = "良好(80-89)";
            else if (score >= 60) level = "一般(60-79)";
            else if (score >= 40) level = "较差(40-59)";
            else level = "危险(0-39)";
            
            statistics[level] = statistics[level].toInt() + 1;
            total++;
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        for (const Facility &facility : facilities) {
            int score = facility.healthScore();
            QString level;
            if (score >= 90) level = "优秀(90-100)";
            else if (score >= 80) level = "良好(80-89)";
            else if (score >= 60) level = "一般(60-79)";
            else if (score >= 40) level = "较差(40-59)";
            else level = "危险(0-39)";
            
            statistics[level] = statistics[level].toInt() + 1;
            total++;
        }
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "健康度等级" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1").arg(total));
    
    // 计算平均健康度
    double avgScore = calculateAverageHealthScore();
    m_summaryLabel->setText(QString("平均健康度: %1").arg(formatNumber(avgScore, 1)));
}

void AssetStatisticsDialog::calculateByLength()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    double totalLength = 0.0;
    
    PipelineDAO dao;
    QVector<Pipeline> pipelines = dao.findAll(10000);
    
    QStringList ranges = {"0-100m", "100-500m", "500-1000m", "1000-2000m", "2000m以上"};
    for (const QString &range : ranges) {
        statistics[range] = 0;
    }
    
    for (const Pipeline &pipeline : pipelines) {
        double length = pipeline.lengthM();
        totalLength += length;
        total++;
        
        QString range;
        if (length < 100) range = "0-100m";
        else if (length < 500) range = "100-500m";
        else if (length < 1000) range = "500-1000m";
        else if (length < 2000) range = "1000-2000m";
        else range = "2000m以上";
        
        statistics[range] = statistics[range].toInt() + 1;
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "长度区间" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1 条管线，总长度: %2 m").arg(total).arg(formatNumber(totalLength, 2)));
    m_summaryLabel->setText(QString("平均长度: %1 m").arg(formatNumber(total > 0 ? totalLength / total : 0, 2)));
}

void AssetStatisticsDialog::calculateByDiameter()
{
    QMap<QString, QVariant> statistics;
    int total = 0;
    
    PipelineDAO dao;
    QVector<Pipeline> pipelines = dao.findAll(10000);
    
    QStringList ranges = {"DN50以下", "DN50-DN100", "DN100-DN200", "DN200-DN500", "DN500以上"};
    for (const QString &range : ranges) {
        statistics[range] = 0;
    }
    
    for (const Pipeline &pipeline : pipelines) {
        int diameter = pipeline.diameterMm();
        total++;
        
        QString range;
        if (diameter < 50) range = "DN50以下";
        else if (diameter < 100) range = "DN50-DN100";
        else if (diameter < 200) range = "DN100-DN200";
        else if (diameter < 500) range = "DN200-DN500";
        else range = "DN500以上";
        
        statistics[range] = statistics[range].toInt() + 1;
    }
    
    // 计算占比
    QMap<QString, QVariant> result;
    for (auto it = statistics.constBegin(); it != statistics.constEnd(); ++it) {
        int count = it.value().toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        QMap<QString, QVariant> item;
        item["count"] = count;
        item["percentage"] = percentage;
        result[it.key()] = item;
    }
    
    updateStatisticsTable(result, QStringList() << "管径区间" << "数量" << "占比(%)");
    m_totalLabel->setText(QString("总计: %1 条管线").arg(total));
    m_summaryLabel->setText(QString("共 %1 个管径区间").arg(statistics.size()));
}

void AssetStatisticsDialog::updateStatisticsTable(const QMap<QString, QVariant> &statistics, const QStringList &headers)
{
    m_statisticsTable->setColumnCount(headers.size());
    m_statisticsTable->setHorizontalHeaderLabels(headers);
    m_statisticsTable->setRowCount(statistics.size());
    
    int row = 0;
    // 按分类名称排序
    QList<QString> keys = statistics.keys();
    std::sort(keys.begin(), keys.end());
    
    for (const QString &key : keys) {
        QMap<QString, QVariant> item = statistics[key].toMap();
        int count = item["count"].toInt();
        double percentage = item["percentage"].toDouble();
        
        m_statisticsTable->setItem(row, 0, new QTableWidgetItem(key));
        m_statisticsTable->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
        m_statisticsTable->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(formatNumber(percentage, 1))));
        
        row++;
    }
}

QString AssetStatisticsDialog::formatNumber(double value, int decimals) const
{
    return QString::number(value, 'f', decimals);
}

QString AssetStatisticsDialog::getBuildYearRange(int year) const
{
    if (year < 1980) return "1980年以前";
    else if (year < 1990) return "1980-1989年";
    else if (year < 2000) return "1990-1999年";
    else if (year < 2010) return "2000-2009年";
    else if (year < 2020) return "2010-2019年";
    else return "2020年及以后";
}

double AssetStatisticsDialog::calculateAverageHealthScore() const
{
    double totalScore = 0.0;
    int count = 0;
    
    if (m_isPipelineStatistics) {
        PipelineDAO dao;
        QVector<Pipeline> pipelines = dao.findAll(10000);
        for (const Pipeline &pipeline : pipelines) {
            totalScore += pipeline.healthScore();
            count++;
        }
    } else {
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        for (const Facility &facility : facilities) {
            totalScore += facility.healthScore();
            count++;
        }
    }
    
    return count > 0 ? totalScore / count : 0.0;
}

void AssetStatisticsDialog::onExportExcelClicked()
{
    // 设置默认导出目录为 docs
    QString defaultDir = QDir::currentPath() + "/docs";
    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath(".");  // 如果目录不存在，创建它
    }
    
    QString defaultFileName = defaultDir + "/资产统计报表_" + QDate::currentDate().toString("yyyyMMdd") + ".csv";
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出Excel", 
        defaultFileName,
        "CSV文件 (*.csv);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 确保文件扩展名是.csv
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) {
        fileName += ".csv";
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", QString("无法创建文件: %1\n%2").arg(fileName).arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out.setGenerateByteOrderMark(true);  // 添加BOM，确保Excel正确识别UTF-8
    
    // 写入标题行
    QString statisticsType = m_isPipelineStatistics ? "管线资产" : "设施资产";
    QString dimensionName = m_dimensionCombo->currentText();
    out << QString("资产统计报表\n");
    out << QString("统计对象: %1\n").arg(statisticsType);
    out << QString("统计维度: %1\n").arg(dimensionName);
    out << QString("生成时间: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    out << QString("总计: %1\n").arg(m_totalLabel->text());
    out << QString("摘要: %1\n").arg(m_summaryLabel->text());
    out << "\n";
    
    // 写入表头
    int columnCount = m_statisticsTable->columnCount();
    for (int col = 0; col < columnCount; col++) {
        QTableWidgetItem *headerItem = m_statisticsTable->horizontalHeaderItem(col);
        QString headerText = headerItem ? headerItem->text() : QString("列%1").arg(col + 1);
        if (col > 0) out << ",";
        out << "\"" << headerText.replace("\"", "\"\"") << "\"";  // 转义引号
    }
    out << "\n";
    
    // 写入数据行
    int rowCount = m_statisticsTable->rowCount();
    for (int row = 0; row < rowCount; row++) {
        for (int col = 0; col < columnCount; col++) {
            QTableWidgetItem *item = m_statisticsTable->item(row, col);
            QString cellText = item ? item->text() : "";
            if (col > 0) out << ",";
            out << "\"" << cellText.replace("\"", "\"\"") << "\"";  // 转义引号
        }
        out << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "成功", QString("Excel文件已导出到:\n%1\n\n提示: CSV文件可以用Excel直接打开").arg(fileName));
}

void AssetStatisticsDialog::onExportPdfClicked()
{
    // 检查权限
    if (!PermissionManager::canExportData()) {
        QMessageBox::warning(this, "权限不足", "您没有权限导出数据。");
        return;
    }
    
    // 设置默认导出目录为 docs
    QString defaultDir = QDir::currentPath() + "/docs";
    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath(".");  // 如果目录不存在，创建它
    }
    
    QString defaultFileName = defaultDir + "/资产统计报表_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf";
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出PDF", 
        defaultFileName,
        "PDF文件 (*.pdf)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 确保文件扩展名是.pdf
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
#else
    printer.setPageSize(QPrinter::A4);
    printer.setPageOrientation(QPrinter::Portrait);
    printer.setPageMargins(20, 20, 20, 20, QPrinter::Millimeter);
#endif
    
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::warning(this, "错误", "无法创建PDF文件");
        return;
    }
    
    // 设置字体
    QFont titleFont("Arial", 16, QFont::Bold);
    QFont headerFont("Arial", 10, QFont::Bold);
    QFont normalFont("Arial", 9);
    QFont smallFont("Arial", 8);
    
    int yPos = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPageLayout pageLayout = printer.pageLayout();
    QRectF pageRect = pageLayout.paintRectPixels(printer.resolution());
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
#else
    int pageWidth = printer.pageRect().width();
    int pageHeight = printer.pageRect().height();
#endif
    int margin = 50;
    int lineHeight = 20;
    int tableRowHeight = 25;
    
    // 绘制标题
    painter.setFont(titleFont);
    painter.drawText(margin, yPos + lineHeight, "资产统计报表");
    yPos += lineHeight * 2;
    
    // 绘制统计信息
    painter.setFont(normalFont);
    QString statisticsType = m_isPipelineStatistics ? "管线资产" : "设施资产";
    QString dimensionName = m_dimensionCombo->currentText();
    painter.drawText(margin, yPos, QString("统计对象: %1").arg(statisticsType));
    yPos += lineHeight;
    painter.drawText(margin, yPos, QString("统计维度: %1").arg(dimensionName));
    yPos += lineHeight;
    painter.drawText(margin, yPos, QString("生成时间: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    yPos += lineHeight;
    painter.drawText(margin, yPos, m_totalLabel->text());
    yPos += lineHeight;
    painter.drawText(margin, yPos, m_summaryLabel->text());
    yPos += lineHeight * 2;
    
    // 绘制表格
    int columnCount = m_statisticsTable->columnCount();
    int rowCount = m_statisticsTable->rowCount();
    
    if (columnCount > 0 && rowCount > 0) {
        // 计算列宽
        QList<int> columnWidths;
        int totalWidth = pageWidth - margin * 2;
        int availableWidth = totalWidth - 20;  // 留出边距
        
        for (int col = 0; col < columnCount; col++) {
            // 根据列数平均分配，第一列稍宽
            if (col == 0) {
                columnWidths.append(availableWidth * 0.4);
            } else {
                columnWidths.append(availableWidth * 0.3);
            }
        }
        
        // 绘制表头
        painter.setFont(headerFont);
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(QBrush(QColor(240, 240, 240)));
        
        int xPos = margin;
        for (int col = 0; col < columnCount; col++) {
            QTableWidgetItem *headerItem = m_statisticsTable->horizontalHeaderItem(col);
            QString headerText = headerItem ? headerItem->text() : QString("列%1").arg(col + 1);
            
            // 绘制表头背景
            painter.drawRect(xPos, yPos, columnWidths[col], tableRowHeight);
            
            // 绘制表头文本
            painter.setPen(QPen(Qt::black, 1));
            painter.drawText(xPos + 5, yPos + tableRowHeight - 5, columnWidths[col] - 10, tableRowHeight,
                            Qt::AlignLeft | Qt::AlignVCenter, headerText);
            
            xPos += columnWidths[col];
        }
        yPos += tableRowHeight;
        
        // 绘制数据行
        painter.setFont(normalFont);
        for (int row = 0; row < rowCount; row++) {
            // 检查是否需要新页面
            if (yPos + tableRowHeight > pageHeight - margin) {
                printer.newPage();
                yPos = margin;
            }
            
            // 交替行颜色
            if (row % 2 == 0) {
                painter.setBrush(QBrush(QColor(250, 250, 250)));
            } else {
                painter.setBrush(QBrush(Qt::white));
            }
            
            xPos = margin;
            for (int col = 0; col < columnCount; col++) {
                QTableWidgetItem *item = m_statisticsTable->item(row, col);
                QString cellText = item ? item->text() : "";
                
                // 绘制单元格背景
                painter.setPen(QPen(QColor(200, 200, 200), 1));
                painter.drawRect(xPos, yPos, columnWidths[col], tableRowHeight);
                
                // 绘制单元格文本
                painter.setPen(QPen(Qt::black, 1));
                painter.drawText(xPos + 5, yPos + tableRowHeight - 5, columnWidths[col] - 10, tableRowHeight,
                                Qt::AlignLeft | Qt::AlignVCenter, cellText);
                
                xPos += columnWidths[col];
            }
            yPos += tableRowHeight;
        }
    }
    
    painter.end();
    
    QMessageBox::information(this, "成功", QString("PDF文件已导出到:\n%1").arg(fileName));
}

