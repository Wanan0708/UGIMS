#include "healthdevicelistdialog.h"
#include "healthassessmentdialog.h"  // 包含 DeviceInfo 定义
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QColor>
#include <QFont>

HealthDeviceListDialog::HealthDeviceListDialog(const QString &healthLevel,
                                               const QList<DeviceInfo> &devices,
                                               QWidget *parent)
    : QDialog(parent)
    , m_healthLevel(healthLevel)
    , m_deviceTable(nullptr)
    , m_closeBtn(nullptr)
{
    setWindowTitle(QString("健康度等级：%1 - 设备列表").arg(healthLevel));
    setMinimumSize(700, 500);
    resize(800, 600);
    
    setupUI();
    populateDeviceList(devices);
}

HealthDeviceListDialog::~HealthDeviceListDialog()
{
}

void HealthDeviceListDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 标题
    QLabel *titleLabel = new QLabel(QString("健康度等级：<b>%1</b>").arg(m_healthLevel), this);
    titleLabel->setStyleSheet("QLabel { font-size: 14pt; padding: 10px; }");
    mainLayout->addWidget(titleLabel);
    
    // 设备列表表格
    m_deviceTable = new QTableWidget(this);
    m_deviceTable->setColumnCount(5);
    m_deviceTable->setHorizontalHeaderLabels(QStringList() << "类型" << "设备编号" << "设备名称" << "健康度分数" << "健康度等级");
    m_deviceTable->horizontalHeader()->setStretchLastSection(true);
    m_deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceTable->verticalHeader()->setVisible(false);
    m_deviceTable->setSortingEnabled(true);
    m_deviceTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_deviceTable);
    
    // 关闭按钮
    QHBoxLayout *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    m_closeBtn = new QPushButton("关闭", this);
    m_closeBtn->setMinimumWidth(100);
    m_closeBtn->setMinimumHeight(35);
    closeLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(closeLayout);
    
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void HealthDeviceListDialog::populateDeviceList(const QList<DeviceInfo> &devices)
{
    m_deviceTable->setRowCount(devices.size());
    
    for (int i = 0; i < devices.size(); i++) {
        const DeviceInfo &device = devices[i];
        
        // 类型
        QTableWidgetItem *typeItem = new QTableWidgetItem(device.type);
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_deviceTable->setItem(i, 0, typeItem);
        
        // 设备编号
        QTableWidgetItem *idItem = new QTableWidgetItem(device.id);
        m_deviceTable->setItem(i, 1, idItem);
        
        // 设备名称
        QTableWidgetItem *nameItem = new QTableWidgetItem(device.name);
        m_deviceTable->setItem(i, 2, nameItem);
        
        // 健康度分数
        QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(device.healthScore));
        scoreItem->setTextAlignment(Qt::AlignCenter);
        m_deviceTable->setItem(i, 3, scoreItem);
        
        // 健康度等级
        QTableWidgetItem *levelItem = new QTableWidgetItem(device.healthLevel);
        levelItem->setTextAlignment(Qt::AlignCenter);
        
        // 根据健康度等级设置颜色
        QColor bgColor;
        if (device.healthLevel == "优秀") {
            bgColor = QColor(0, 200, 0);
        } else if (device.healthLevel == "良好") {
            bgColor = QColor(100, 200, 100);
        } else if (device.healthLevel == "一般") {
            bgColor = QColor(255, 200, 0);
        } else if (device.healthLevel == "较差") {
            bgColor = QColor(255, 150, 0);
        } else if (device.healthLevel == "危险") {
            bgColor = QColor(255, 0, 0);
        }
        levelItem->setBackground(bgColor);
        levelItem->setForeground(QColor(255, 255, 255));
        m_deviceTable->setItem(i, 4, levelItem);
    }
    
    m_deviceTable->resizeColumnsToContents();
}

