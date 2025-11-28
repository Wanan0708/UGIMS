#include "analysispanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

AnalysisPanel::AnalysisPanel(QWidget *parent)
    : QWidget(parent)
    , m_burstAnalyzer(new BurstAnalyzer(this))
    , m_connectivityAnalyzer(new ConnectivityAnalyzer(this))
{
    setupUI();
    
    // 连接信号
    connect(m_burstAnalyzer, &BurstAnalyzer::burstAnalysisFinished,
            this, &AnalysisPanel::onBurstAnalysisFinished);
    
    connect(m_connectivityAnalyzer, &ConnectivityAnalyzer::connectivityAnalysisFinished,
            this, &AnalysisPanel::onConnectivityAnalysisFinished);
}

AnalysisPanel::~AnalysisPanel()
{
}

void AnalysisPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 创建各个分析区域
    createBurstAnalysisSection();
    createConnectivitySection();
    createResultSection();
    
    QGroupBox *burstGroup = new QGroupBox("🚰 爆管影响分析");
    QVBoxLayout *burstLayout = new QVBoxLayout;
    burstLayout->addWidget(new QLabel("搜索半径 (米):"));
    m_searchRadiusSpinBox = new QDoubleSpinBox;
    m_searchRadiusSpinBox->setRange(100, 5000);
    m_searchRadiusSpinBox->setValue(500);
    m_searchRadiusSpinBox->setSingleStep(100);
    burstLayout->addWidget(m_searchRadiusSpinBox);
    m_burstAnalysisBtn = new QPushButton("执行爆管分析");
    connect(m_burstAnalysisBtn, &QPushButton::clicked, this, &AnalysisPanel::onBurstAnalysisClicked);
    burstLayout->addWidget(m_burstAnalysisBtn);
    burstGroup->setLayout(burstLayout);
    mainLayout->addWidget(burstGroup);
    
    // 连通性分析
    QGroupBox *connGroup = new QGroupBox("🔗 连通性分析");
    QVBoxLayout *connLayout = new QVBoxLayout;
    m_upstreamBtn = new QPushButton("上游追踪");
    m_downstreamBtn = new QPushButton("下游追踪");
    m_shortestPathBtn = new QPushButton("最短路径");
    connect(m_upstreamBtn, &QPushButton::clicked, this, &AnalysisPanel::onUpstreamTraceClicked);
    connect(m_downstreamBtn, &QPushButton::clicked, this, &AnalysisPanel::onDownstreamTraceClicked);
    connect(m_shortestPathBtn, &QPushButton::clicked, this, &AnalysisPanel::onShortestPathClicked);
    connLayout->addWidget(m_upstreamBtn);
    connLayout->addWidget(m_downstreamBtn);
    connLayout->addWidget(m_shortestPathBtn);
    connGroup->setLayout(connLayout);
    mainLayout->addWidget(connGroup);
    
    // 结果显示
    QGroupBox *resultGroup = new QGroupBox("📊 分析结果");
    QVBoxLayout *resultLayout = new QVBoxLayout;
    m_resultText = new QTextEdit;
    m_resultText->setReadOnly(true);
    m_resultText->setMinimumHeight(200);
    resultLayout->addWidget(m_resultText);
    
    QHBoxLayout *btnLayout = new QHBoxLayout;
    m_clearBtn = new QPushButton("清除");
    m_exportBtn = new QPushButton("导出报告");
    connect(m_clearBtn, &QPushButton::clicked, this, &AnalysisPanel::onClearClicked);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_exportBtn);
    resultLayout->addLayout(btnLayout);
    
    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);
    
    mainLayout->addStretch();
}

void AnalysisPanel::createBurstAnalysisSection()
{
    // 已在setupUI中创建
}

void AnalysisPanel::createConnectivitySection()
{
    // 已在setupUI中创建
}

void AnalysisPanel::createResultSection()
{
    // 已在setupUI中创建
}

void AnalysisPanel::setAnalysisPoint(const QPointF &point)
{
    m_analysisPoint = point;
    qDebug() << "[AnalysisPanel] Analysis point set to:" << point;
}

void AnalysisPanel::onBurstAnalysisClicked()
{
    if (m_analysisPoint.isNull()) {
        QMessageBox::warning(this, "提示", "请先在地图上选择爆管位置");
        return;
    }
    
    m_resultText->append(QString("<b>==== 爆管影响分析 ====</b><br>时间: %1<br>位置: (%2, %3)")
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                         .arg(m_analysisPoint.x())
                         .arg(m_analysisPoint.y()));
    
    // 设置参数
    m_burstAnalyzer->setSearchRadius(m_searchRadiusSpinBox->value());
    
    // 执行分析
    m_burstAnalyzer->analyzeBurstAsync(m_analysisPoint);
    m_burstAnalysisBtn->setEnabled(false);
    m_burstAnalysisBtn->setText("分析中...");
}

void AnalysisPanel::onUpstreamTraceClicked()
{
    if (m_analysisPoint.isNull()) {
        QMessageBox::warning(this, "提示", "请先在地图上选择起点");
        return;
    }
    
    m_resultText->append("<b>==== 上游追踪 ====</b>");
    ConnectivityResult result = m_connectivityAnalyzer->traceUpstream(m_analysisPoint);
    onConnectivityAnalysisFinished(result);
}

void AnalysisPanel::onDownstreamTraceClicked()
{
    if (m_analysisPoint.isNull()) {
        QMessageBox::warning(this, "提示", "请先在地图上选择起点");
        return;
    }
    
    m_resultText->append("<b>==== 下游追踪 ====</b>");
    ConnectivityResult result = m_connectivityAnalyzer->traceDownstream(m_analysisPoint);
    onConnectivityAnalysisFinished(result);
}

void AnalysisPanel::onShortestPathClicked()
{
    // TODO: 需要选择两个点
    QMessageBox::information(this, "提示", "最短路径分析需要选择起点和终点");
}

void AnalysisPanel::onBurstAnalysisFinished(const BurstAnalysisResult &result)
{
    m_burstAnalysisBtn->setEnabled(true);
    m_burstAnalysisBtn->setText("执行爆管分析");
    
    if (result.success) {
        m_resultText->append(formatBurstResult(result));
        emit analysisCompleted("burst", true);
    } else {
        m_resultText->append(QString("<font color='red'>分析失败: %1</font><br>").arg(result.message));
        emit analysisCompleted("burst", false);
    }
}

void AnalysisPanel::onConnectivityAnalysisFinished(const ConnectivityResult &result)
{
    if (result.success) {
        m_resultText->append(formatConnectivityResult(result));
        emit analysisCompleted("connectivity", true);
    } else {
        m_resultText->append(QString("<font color='red'>分析失败: %1</font><br>").arg(result.message));
        emit analysisCompleted("connectivity", false);
    }
}

void AnalysisPanel::onClearClicked()
{
    m_resultText->clear();
}

QString AnalysisPanel::formatBurstResult(const BurstAnalysisResult &result)
{
    QString html;
    
    html += QString("<font color='green'><b>✓ 分析完成</b></font><br>");
    html += QString("<b>管线信息:</b><br>");
    html += QString("  管线ID: %1<br>").arg(result.pipelineId);
    html += QString("  管径: %.0f mm<br>").arg(result.pipelineDiameter);
    
    html += QString("<br><b>影响范围:</b><br>");
    html += QString("  受影响管线: %1 条<br>").arg(result.affectedPipelines.size());
    html += QString("  需关闭阀门: %1 个<br>").arg(result.affectedValves.size());
    html += QString("  影响用户: 约 %1 户<br>").arg(result.estimatedAffectedUsers);
    html += QString("  影响面积: %.2f km²<br>").arg(result.affectedAreaSize / 1000000.0);
    
    html += QString("<br><b>维修信息:</b><br>");
    html += QString("  优先级: %1/5<br>").arg(result.repairPriority);
    html += QString("  预计耗时: %.1f 小时<br>").arg(result.estimatedRepairTime);
    
    html += QString("<br><b>建议操作:</b><br>");
    for (const QString &action : result.suggestedActions) {
        html += QString("  %1<br>").arg(action);
    }
    
    html += "<br>";
    return html;
}

QString AnalysisPanel::formatConnectivityResult(const ConnectivityResult &result)
{
    QString html;
    
    html += QString("<font color='green'><b>✓ 追踪完成</b></font><br>");
    html += QString("<b>路径信息:</b><br>");
    html += QString("  节点数: %1<br>").arg(result.nodeCount);
    html += QString("  管线数: %1<br>").arg(result.pathPipelines.size());
    html += QString("  总长度: %.0f 米<br>").arg(result.totalLength);
    html += QString("  连通性: %1<br>").arg(result.isConnected ? "连通" : "不连通");
    
    html += QString("<br><b>路径节点:</b><br>");
    for (const QString &node : result.pathNodes) {
        html += QString("  → %1<br>").arg(node);
    }
    
    html += "<br>";
    return html;
}

