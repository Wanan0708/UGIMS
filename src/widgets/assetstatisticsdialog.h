#ifndef ASSETSTATISTICSDIALOG_H
#define ASSETSTATISTICSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QLabel>

class QVBoxLayout;
class QHBoxLayout;

/**
 * @brief 资产统计报表对话框
 * 提供多维度统计和报表导出功能
 */
class AssetStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetStatisticsDialog(QWidget *parent = nullptr);
    ~AssetStatisticsDialog();

private slots:
    void onRefreshClicked();
    void onExportExcelClicked();
    void onExportPdfClicked();
    void onStatisticsTypeChanged(int index);
    void onDimensionChanged(int index);

private:
    void setupUI();
    void setupStatisticsPanel();
    void setupExportPanel();
    void refreshStatistics();
    
    // 统计方法
    void calculateByType();           // 按类型统计
    void calculateByStatus();        // 按状态统计
    void calculateByMaterial();      // 按材质统计
    void calculateByBuildYear();     // 按建设年代统计
    void calculateByHealthLevel();   // 按健康度等级统计
    void calculateByLength();        // 按长度区间统计（管线）
    void calculateByDiameter();      // 按管径区间统计（管线）
    
    // 辅助方法
    void updateStatisticsTable(const QMap<QString, QVariant> &statistics, const QStringList &headers);
    QString formatNumber(double value, int decimals = 2) const;
    QString getBuildYearRange(int year) const;

private:
    // UI组件
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // 统计类型选择
    QComboBox *m_statisticsTypeCombo;  // 管线/设施
    QComboBox *m_dimensionCombo;       // 统计维度
    
    // 统计结果表格
    QTableWidget *m_statisticsTable;
    
    // 统计信息标签
    QLabel *m_totalLabel;
    QLabel *m_summaryLabel;
    
    // 导出按钮
    QPushButton *m_refreshBtn;
    QPushButton *m_exportExcelBtn;
    QPushButton *m_exportPdfBtn;
    QPushButton *m_closeBtn;
    
    // 导出布局
    QHBoxLayout *m_exportLayout;
    
    // 数据
    bool m_isPipelineStatistics;  // true=管线统计, false=设施统计
    QString m_currentDimension;   // 当前统计维度
    
    // 辅助方法
    double calculateAverageHealthScore() const;
};

#endif // ASSETSTATISTICSDIALOG_H

