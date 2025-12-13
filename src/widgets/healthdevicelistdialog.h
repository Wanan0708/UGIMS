#ifndef HEALTHDEVICELISTDIALOG_H
#define HEALTHDEVICELISTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QString>

// 前向声明 DeviceInfo（定义在 healthassessmentdialog.h 中）
struct DeviceInfo;

/**
 * @brief 健康度设备列表对话框
 * 显示指定健康度等级下的所有设备
 */
class HealthDeviceListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HealthDeviceListDialog(const QString &healthLevel, 
                                    const QList<DeviceInfo> &devices,
                                    QWidget *parent = nullptr);
    ~HealthDeviceListDialog();

private:
    void setupUI();
    void populateDeviceList(const QList<DeviceInfo> &devices);
    
    QString m_healthLevel;
    QTableWidget *m_deviceTable;
    QPushButton *m_closeBtn;
};

#endif // HEALTHDEVICELISTDIALOG_H

