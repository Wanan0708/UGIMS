#ifndef ASSETMANAGERDIALOG_H
#define ASSETMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>

class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;

/**
 * @brief 资产管理对话框
 * 统一管理管线和设施资产
 */
class AssetManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetManagerDialog(QWidget *parent = nullptr);
    ~AssetManagerDialog();

signals:
    void pipelineModified(const class Pipeline &pipeline, int originalId);
    void facilityModified(const class Facility &facility, int originalId);

private slots:
    void onRefreshClicked();
    void onViewClicked();
    void onEditClicked();
    void onStatisticsClicked();
    void onFilterChanged();
    void onTabChanged(int index);
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupFilterPanel();
    void setupTables();
    void loadPipelines();
    void loadFacilities();
    void refreshPipelineTable();
    void refreshFacilityTable();
    
    QString getStatusDisplayName(const QString &status);
    QString getTypeDisplayName(const QString &type, bool isPipeline);

private:
    // UI组件
    QVBoxLayout *m_mainLayout;
    QGroupBox *m_filterGroup;
    QHBoxLayout *m_filterLayout;
    
    // 筛选控件
    QComboBox *m_typeFilter;
    QComboBox *m_statusFilter;
    QLineEdit *m_searchEdit;
    QPushButton *m_refreshBtn;
    
    // 操作按钮
    QPushButton *m_viewBtn;
    QPushButton *m_editBtn;
    QPushButton *m_statisticsBtn;
    QPushButton *m_closeBtn;
    
    // 标签页和表格
    QTabWidget *m_tabWidget;
    QTableWidget *m_pipelineTable;
    QTableWidget *m_facilityTable;
    
    // 数据
    int m_currentTabIndex;
};

#endif // ASSETMANAGERDIALOG_H

