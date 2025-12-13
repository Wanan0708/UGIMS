#ifndef ANALYSISPANEL_H
#define ANALYSISPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "analysis/burstanalyzer.h"
#include "analysis/connectivityanalyzer.h"

/**
 * @brief 空间分析工具面板
 * 
 * 提供爆管分析、连通性分析等工具的UI界面
 */
class AnalysisPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisPanel(QWidget *parent = nullptr);
    ~AnalysisPanel();

    /**
     * @brief 设置当前分析点
     */
    void setAnalysisPoint(const QPointF &point);

signals:
    /**
     * @brief 请求在地图上显示分析结果
     */
    void requestShowResult(const QPolygonF &area, const QList<QPointF> &points);
    
    /**
     * @brief 分析完成信号
     */
    void analysisCompleted(const QString &type, bool success);
    
    /**
     * @brief 请求开始最短路径分析（需要选择两点）
     */
    void requestShortestPathAnalysis();

private slots:
    /**
     * @brief 执行爆管分析
     */
    void onBurstAnalysisClicked();
    
    /**
     * @brief 执行上游追踪
     */
    void onUpstreamTraceClicked();
    
    /**
     * @brief 执行下游追踪
     */
    void onDownstreamTraceClicked();
    
    /**
     * @brief 执行最短路径分析
     */
    void onShortestPathClicked();
    
    /**
     * @brief 显示爆管分析结果
     */
    void onBurstAnalysisFinished(const BurstAnalysisResult &result);
    
    /**
     * @brief 显示连通性分析结果
     */
    void onConnectivityAnalysisFinished(const ConnectivityResult &result);
    
    /**
     * @brief 清除结果
     */
    void onClearClicked();

private:
    void setupUI();
    void createBurstAnalysisSection();
    void createConnectivitySection();
    void createResultSection();
    
    QString formatBurstResult(const BurstAnalysisResult &result);
    QString formatConnectivityResult(const ConnectivityResult &result);

private:
    // 分析器
    BurstAnalyzer *m_burstAnalyzer;
    ConnectivityAnalyzer *m_connectivityAnalyzer;
    
    // 当前分析点
    QPointF m_analysisPoint;
    
    // UI组件 - 爆管分析
    QComboBox *m_pipelineTypeCombo;
    QDoubleSpinBox *m_searchRadiusSpinBox;
    QPushButton *m_burstAnalysisBtn;
    
    // UI组件 - 连通性分析
    QComboBox *m_traceTypeCombo;
    QSpinBox *m_maxDepthSpinBox;
    QPushButton *m_upstreamBtn;
    QPushButton *m_downstreamBtn;
    QPushButton *m_shortestPathBtn;
    
    // 结果显示
    QTextEdit *m_resultText;
    QPushButton *m_clearBtn;
    QPushButton *m_exportBtn;
};

#endif // ANALYSISPANEL_H

