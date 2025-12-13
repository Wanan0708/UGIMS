#ifndef HEALTHASSESSMENTDIALOG_H
#define HEALTHASSESSMENTDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>
#include <QList>

class HealthAssessmentAnalyzer;
class Pipeline;
class Facility;

// 设备信息结构
struct DeviceInfo {
    QString type;        // "管线" 或 "设施"
    QString id;          // 设备编号
    QString name;        // 设备名称
    int healthScore;     // 健康度分数
    QString healthLevel; // 健康度等级
    
    DeviceInfo() : healthScore(100) {}
};

/**
 * @brief 健康度评估对话框
 */
class HealthAssessmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HealthAssessmentDialog(QWidget *parent = nullptr);
    ~HealthAssessmentDialog();

private slots:
    void onAssessPipelinesClicked();
    void onAssessFacilitiesClicked();
    void onAssessAllClicked();
    void onAssessmentProgress(int current, int total);
    void onAssessmentComplete(int pipelineCount, int facilityCount);
    void onCloseClicked();
    void onViewDevicesClicked(const QString &healthLevel);

private:
    void setupUI();
    void updateStatistics(const QMap<QString, int> &statistics);
    QList<DeviceInfo> getDevicesByLevel(const QString &level) const;
    
    HealthAssessmentAnalyzer *m_analyzer;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTableWidget *m_statisticsTable;
    QPushButton *m_assessPipelinesBtn;
    QPushButton *m_assessFacilitiesBtn;
    QPushButton *m_assessAllBtn;
    QPushButton *m_closeBtn;
    
    bool m_isAssessing;
    QList<DeviceInfo> m_allDevices;  // 存储所有评估过的设备
};

#endif // HEALTHASSESSMENTDIALOG_H

