#include "healthassessmentdialog.h"
#include "analysis/healthassessmentanalyzer.h"
#include "core/models/pipeline.h"
#include "core/models/facility.h"
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "widgets/healthdevicelistdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QFont>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QWidget>

HealthAssessmentDialog::HealthAssessmentDialog(QWidget *parent)
    : QDialog(parent)
    , m_analyzer(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_statisticsTable(nullptr)
    , m_assessPipelinesBtn(nullptr)
    , m_assessFacilitiesBtn(nullptr)
    , m_assessAllBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_isAssessing(false)
{
    setWindowTitle("管网健康度评估");
    setMinimumSize(500, 400);
    resize(600, 450);
    
    m_analyzer = new HealthAssessmentAnalyzer(this);
    
    setupUI();
    
    // 连接信号
    connect(m_analyzer, &HealthAssessmentAnalyzer::assessmentProgress,
            this, &HealthAssessmentDialog::onAssessmentProgress);
    connect(m_analyzer, &HealthAssessmentAnalyzer::assessmentComplete,
            this, &HealthAssessmentDialog::onAssessmentComplete);
}

HealthAssessmentDialog::~HealthAssessmentDialog()
{
}

void HealthAssessmentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 说明标签
    QLabel *infoLabel = new QLabel(
        "健康度评估基于以下因素：\n"
        "• 建设年限（30%）：基于设计寿命和使用年限\n"
        "• 材质类型（25%）：不同材质的耐久性\n"
        "• 巡检/维护记录（20%）：巡检及时性和完整性\n"
        "• 运行状态（15%）：当前运行状态\n"
        "• 当前健康度（10%）：已有健康度分数",
        this
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; border-radius: 5px; }");
    mainLayout->addWidget(infoLabel);
    
    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_assessPipelinesBtn = new QPushButton("评估所有管线", this);
    m_assessFacilitiesBtn = new QPushButton("评估所有设施", this);
    m_assessAllBtn = new QPushButton("评估全部", this);
    
    m_assessPipelinesBtn->setMinimumHeight(35);
    m_assessFacilitiesBtn->setMinimumHeight(35);
    m_assessAllBtn->setMinimumHeight(35);
    
    btnLayout->addWidget(m_assessPipelinesBtn);
    btnLayout->addWidget(m_assessFacilitiesBtn);
    btnLayout->addWidget(m_assessAllBtn);
    btnLayout->addStretch();
    
    mainLayout->addLayout(btnLayout);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    // 状态标签
    m_statusLabel = new QLabel("准备就绪", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);
    
    // 统计表格
    m_statisticsTable = new QTableWidget(this);
    m_statisticsTable->setColumnCount(3);
    m_statisticsTable->setHorizontalHeaderLabels(QStringList() << "健康等级" << "数量" << "操作");
    m_statisticsTable->horizontalHeader()->setStretchLastSection(false);
    m_statisticsTable->setColumnWidth(0, 120);
    m_statisticsTable->setColumnWidth(1, 100);
    m_statisticsTable->setColumnWidth(2, 100);
    m_statisticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statisticsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statisticsTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(m_statisticsTable);
    
    // 关闭按钮
    QHBoxLayout *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    m_closeBtn = new QPushButton("关闭", this);
    m_closeBtn->setMinimumWidth(100);
    m_closeBtn->setMinimumHeight(35);
    closeLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(closeLayout);
    
    // 连接信号
    connect(m_assessPipelinesBtn, &QPushButton::clicked, this, &HealthAssessmentDialog::onAssessPipelinesClicked);
    connect(m_assessFacilitiesBtn, &QPushButton::clicked, this, &HealthAssessmentDialog::onAssessFacilitiesClicked);
    connect(m_assessAllBtn, &QPushButton::clicked, this, &HealthAssessmentDialog::onAssessAllClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &HealthAssessmentDialog::onCloseClicked);
}

void HealthAssessmentDialog::onAssessPipelinesClicked()
{
    if (m_isAssessing) {
        QMessageBox::warning(this, "提示", "评估正在进行中，请稍候...");
        return;
    }
    
    m_isAssessing = true;
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText("正在评估管线健康度...");
    m_assessPipelinesBtn->setEnabled(false);
    m_assessFacilitiesBtn->setEnabled(false);
    m_assessAllBtn->setEnabled(false);
    
    m_allDevices.clear();
    
    // 异步执行评估
    QFuture<QMap<QString, int>> future = QtConcurrent::run([this]() {
        QMap<QString, int> statistics;
        statistics["优秀"] = 0;
        statistics["良好"] = 0;
        statistics["一般"] = 0;
        statistics["较差"] = 0;
        statistics["危险"] = 0;
        
        PipelineDAO dao;
        QStringList types = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
        QVector<Pipeline> pipelines;
        for (const QString &type : types) {
            QVector<Pipeline> typePipelines = dao.findByType(type, 10000);
            pipelines.append(typePipelines);
        }
        
        int total = pipelines.size();
        int current = 0;
        
        for (const Pipeline &pipeline : pipelines) {
            HealthAssessmentResult result = m_analyzer->assessPipeline(pipeline);
            
            Pipeline updatedPipeline = pipeline;
            updatedPipeline.setHealthScore(result.score);
            dao.update(updatedPipeline, pipeline.id());
            
            statistics[result.level]++;
            
            DeviceInfo device;
            device.type = "管线";
            device.id = pipeline.pipelineId();
            device.name = pipeline.pipelineName();
            device.healthScore = result.score;
            device.healthLevel = result.level;
            m_allDevices.append(device);
            
            current++;
            emit m_analyzer->assessmentProgress(current, total);
        }
        
        emit m_analyzer->assessmentComplete(total, 0);
        return statistics;
    });
    
    QFutureWatcher<QMap<QString, int>> *watcher = new QFutureWatcher<QMap<QString, int>>(this);
    connect(watcher, &QFutureWatcher<QMap<QString, int>>::finished, [this, watcher]() {
        QMap<QString, int> statistics = watcher->result();
        updateStatistics(statistics);
        m_isAssessing = false;
        m_progressBar->setVisible(false);
        m_statusLabel->setText("管线评估完成");
        m_assessPipelinesBtn->setEnabled(true);
        m_assessFacilitiesBtn->setEnabled(true);
        m_assessAllBtn->setEnabled(true);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void HealthAssessmentDialog::onAssessFacilitiesClicked()
{
    if (m_isAssessing) {
        QMessageBox::warning(this, "提示", "评估正在进行中，请稍候...");
        return;
    }
    
    m_isAssessing = true;
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText("正在评估设施健康度...");
    m_assessPipelinesBtn->setEnabled(false);
    m_assessFacilitiesBtn->setEnabled(false);
    m_assessAllBtn->setEnabled(false);
    
    m_allDevices.clear();
    
    // 异步执行评估
    QFuture<QMap<QString, int>> future = QtConcurrent::run([this]() {
        QMap<QString, int> statistics;
        statistics["优秀"] = 0;
        statistics["良好"] = 0;
        statistics["一般"] = 0;
        statistics["较差"] = 0;
        statistics["危险"] = 0;
        
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        int total = facilities.size();
        int current = 0;
        
        for (const Facility &facility : facilities) {
            HealthAssessmentResult result = m_analyzer->assessFacility(facility);
            
            Facility updatedFacility = facility;
            updatedFacility.setHealthScore(result.score);
            dao.update(updatedFacility, facility.id());
            
            statistics[result.level]++;
            
            DeviceInfo device;
            device.type = "设施";
            device.id = facility.facilityId();
            device.name = facility.facilityName();
            device.healthScore = result.score;
            device.healthLevel = result.level;
            m_allDevices.append(device);
            
            current++;
            emit m_analyzer->assessmentProgress(current, total);
        }
        
        emit m_analyzer->assessmentComplete(0, total);
        return statistics;
    });
    
    QFutureWatcher<QMap<QString, int>> *watcher = new QFutureWatcher<QMap<QString, int>>(this);
    connect(watcher, &QFutureWatcher<QMap<QString, int>>::finished, [this, watcher]() {
        QMap<QString, int> statistics = watcher->result();
        updateStatistics(statistics);
        m_isAssessing = false;
        m_progressBar->setVisible(false);
        m_statusLabel->setText("设施评估完成");
        m_assessPipelinesBtn->setEnabled(true);
        m_assessFacilitiesBtn->setEnabled(true);
        m_assessAllBtn->setEnabled(true);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void HealthAssessmentDialog::onAssessAllClicked()
{
    if (m_isAssessing) {
        QMessageBox::warning(this, "提示", "评估正在进行中，请稍候...");
        return;
    }
    
    m_isAssessing = true;
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText("正在评估所有资产健康度...");
    m_assessPipelinesBtn->setEnabled(false);
    m_assessFacilitiesBtn->setEnabled(false);
    m_assessAllBtn->setEnabled(false);
    m_allDevices.clear();
    
    // 异步执行评估（先评估管线，再评估设施）
    QFuture<QMap<QString, int>> pipelineFuture = QtConcurrent::run([this]() {
        QMap<QString, int> statistics;
        statistics["优秀"] = 0;
        statistics["良好"] = 0;
        statistics["一般"] = 0;
        statistics["较差"] = 0;
        statistics["危险"] = 0;
        
        PipelineDAO dao;
        QStringList types = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
        QVector<Pipeline> pipelines;
        for (const QString &type : types) {
            QVector<Pipeline> typePipelines = dao.findByType(type, 10000);
            pipelines.append(typePipelines);
        }
        
        int total = pipelines.size();
        int current = 0;
        
        for (const Pipeline &pipeline : pipelines) {
            HealthAssessmentResult result = m_analyzer->assessPipeline(pipeline);
            
            Pipeline updatedPipeline = pipeline;
            updatedPipeline.setHealthScore(result.score);
            dao.update(updatedPipeline, pipeline.id());
            
            statistics[result.level]++;
            
            DeviceInfo device;
            device.type = "管线";
            device.id = pipeline.pipelineId();
            device.name = pipeline.pipelineName();
            device.healthScore = result.score;
            device.healthLevel = result.level;
            m_allDevices.append(device);
            
            current++;
            emit m_analyzer->assessmentProgress(current, total * 2); // 总进度是管线+设施
        }
        
        return statistics;
    });
    
    QFuture<QMap<QString, int>> facilityFuture = QtConcurrent::run([this]() {
        QMap<QString, int> statistics;
        statistics["优秀"] = 0;
        statistics["良好"] = 0;
        statistics["一般"] = 0;
        statistics["较差"] = 0;
        statistics["危险"] = 0;
        
        FacilityDAO dao;
        QVector<Facility> facilities = dao.findAll(10000);
        
        int total = facilities.size();
        int current = 0;
        
        for (const Facility &facility : facilities) {
            HealthAssessmentResult result = m_analyzer->assessFacility(facility);
            
            Facility updatedFacility = facility;
            updatedFacility.setHealthScore(result.score);
            dao.update(updatedFacility, facility.id());
            
            statistics[result.level]++;
            
            DeviceInfo device;
            device.type = "设施";
            device.id = facility.facilityId();
            device.name = facility.facilityName();
            device.healthScore = result.score;
            device.healthLevel = result.level;
            m_allDevices.append(device);
            
            current++;
            emit m_analyzer->assessmentProgress(current, total * 2);
        }
        
        return statistics;
    });
    
    // 等待两个评估都完成
    QFutureWatcher<QMap<QString, int>> *pipelineWatcher = new QFutureWatcher<QMap<QString, int>>(this);
    QFutureWatcher<QMap<QString, int>> *facilityWatcher = new QFutureWatcher<QMap<QString, int>>(this);
    
    QMap<QString, int> *pipelineStats = new QMap<QString, int>();
    QMap<QString, int> *facilityStats = new QMap<QString, int>();
    int *completedCount = new int(0);
    
    auto checkComplete = [this, pipelineStats, facilityStats, completedCount, pipelineWatcher, facilityWatcher]() {
        (*completedCount)++;
        if (*completedCount == 2) {
            // 合并统计
            QMap<QString, int> totalStats;
            QStringList levels = {"优秀", "良好", "一般", "较差", "危险"};
            for (const QString &level : levels) {
                totalStats[level] = pipelineStats->value(level, 0) + facilityStats->value(level, 0);
            }
            
            updateStatistics(totalStats);
            m_isAssessing = false;
            m_progressBar->setVisible(false);
            m_statusLabel->setText("全部评估完成");
            m_assessPipelinesBtn->setEnabled(true);
            m_assessFacilitiesBtn->setEnabled(true);
            m_assessAllBtn->setEnabled(true);
            
            delete pipelineStats;
            delete facilityStats;
            delete completedCount;
            pipelineWatcher->deleteLater();
            facilityWatcher->deleteLater();
        }
    };
    
    connect(pipelineWatcher, &QFutureWatcher<QMap<QString, int>>::finished, [pipelineWatcher, pipelineStats, checkComplete]() {
        *pipelineStats = pipelineWatcher->result();
        checkComplete();
    });
    
    connect(facilityWatcher, &QFutureWatcher<QMap<QString, int>>::finished, [facilityWatcher, facilityStats, checkComplete]() {
        *facilityStats = facilityWatcher->result();
        checkComplete();
    });
    
    pipelineWatcher->setFuture(pipelineFuture);
    facilityWatcher->setFuture(facilityFuture);
}

void HealthAssessmentDialog::onAssessmentProgress(int current, int total)
{
    if (total > 0) {
        int percent = (current * 100) / total;
        m_progressBar->setValue(percent);
        m_statusLabel->setText(QString("评估进度: %1/%2 (%3%)")
                               .arg(current).arg(total).arg(percent));
    }
}

void HealthAssessmentDialog::onAssessmentComplete(int pipelineCount, int facilityCount)
{
    qDebug() << "Assessment complete - Pipelines:" << pipelineCount << "Facilities:" << facilityCount;
}

void HealthAssessmentDialog::updateStatistics(const QMap<QString, int> &statistics)
{
    m_statisticsTable->setRowCount(5);
    
    QStringList levels = {"优秀", "良好", "一般", "较差", "危险"};
    QList<QColor> colors = {
        QColor(0, 200, 0),      // 优秀 - 绿色
        QColor(100, 200, 100),  // 良好 - 浅绿色
        QColor(255, 200, 0),    // 一般 - 黄色
        QColor(255, 150, 0),    // 较差 - 橙色
        QColor(255, 0, 0)       // 危险 - 红色
    };
    
    int total = 0;
    for (int i = 0; i < levels.size(); i++) {
        QString level = levels[i];
        int count = statistics.value(level, 0);
        total += count;
        
        // 健康等级列
        QTableWidgetItem *levelItem = new QTableWidgetItem(level);
        levelItem->setBackground(colors[i]);
        levelItem->setForeground(QColor(255, 255, 255));
        levelItem->setTextAlignment(Qt::AlignCenter);
        m_statisticsTable->setItem(i, 0, levelItem);
        
        // 数量列
        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(count));
        countItem->setTextAlignment(Qt::AlignCenter);
        m_statisticsTable->setItem(i, 1, countItem);
        
        // 操作列 - 添加查看按钮
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 2, 2, 2);
        actionLayout->setSpacing(2);
        
        QPushButton *viewBtn = new QPushButton("查看", actionWidget);
        viewBtn->setMinimumHeight(25);
        viewBtn->setMaximumHeight(25);
        viewBtn->setEnabled(count > 0);  // 如果没有设备，禁用按钮
        
        // 使用 lambda 捕获 level
        connect(viewBtn, &QPushButton::clicked, [this, level]() {
            onViewDevicesClicked(level);
        });
        
        actionLayout->addWidget(viewBtn);
        actionLayout->setAlignment(Qt::AlignCenter);
        m_statisticsTable->setCellWidget(i, 2, actionWidget);
    }
    
    // 添加总计行
    m_statisticsTable->setRowCount(6);
    QTableWidgetItem *totalLabelItem = new QTableWidgetItem("总计");
    totalLabelItem->setFont(QFont("Arial", 10, QFont::Bold));
    totalLabelItem->setTextAlignment(Qt::AlignCenter);
    m_statisticsTable->setItem(5, 0, totalLabelItem);
    
    QTableWidgetItem *totalCountItem = new QTableWidgetItem(QString::number(total));
    totalCountItem->setFont(QFont("Arial", 10, QFont::Bold));
    totalCountItem->setTextAlignment(Qt::AlignCenter);
    m_statisticsTable->setItem(5, 1, totalCountItem);
    
    // 总计行的操作列为空
    m_statisticsTable->setItem(5, 2, new QTableWidgetItem(""));
}

QList<DeviceInfo> HealthAssessmentDialog::getDevicesByLevel(const QString &level) const
{
    QList<DeviceInfo> result;
    for (const DeviceInfo &device : m_allDevices) {
        if (device.healthLevel == level) {
            result.append(device);
        }
    }
    return result;
}

void HealthAssessmentDialog::onViewDevicesClicked(const QString &healthLevel)
{
    QList<DeviceInfo> devices = getDevicesByLevel(healthLevel);
    
    if (devices.isEmpty()) {
        QMessageBox::information(this, "提示", QString("健康度等级 '%1' 下没有设备").arg(healthLevel));
        return;
    }
    
    // 打开设备列表对话框
    HealthDeviceListDialog dialog(healthLevel, devices, this);
    dialog.exec();
}

void HealthAssessmentDialog::onCloseClicked()
{
    if (m_isAssessing) {
        int ret = QMessageBox::question(this, "确认", "评估正在进行中，确定要关闭吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
    }
    accept();
}

