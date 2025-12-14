#include "facilityeditdialog.h"
#include "core/utils/idgenerator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QWidget>
#include <QDebug>

FacilityEditDialog::FacilityEditDialog(QWidget *parent, const Facility &facility)
    : QDialog(parent)
    , m_facility(facility)
{
    setWindowTitle(facility.isValid() ? tr("编辑设施") : tr("新建设施"));
    setMinimumSize(550, 650);
    resize(600, 750);
    setupUI();
    populateFields(facility);
}

FacilityEditDialog::~FacilityEditDialog()
{
}

void FacilityEditDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 创建滚动区域的内容widget
    QWidget *scrollContent = new QWidget();
    auto *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(15);
    scrollLayout->setContentsMargins(10, 10, 10, 10);
    
    // 基本信息组
    auto *basicGroup = new QGroupBox(tr("基本信息"), this);
    auto *basicLayout = new QFormLayout(basicGroup);
    basicLayout->setSpacing(10);
    basicLayout->setContentsMargins(15, 15, 15, 15);
    
    m_facilityIdEdit = new QLineEdit(this);
    m_facilityIdEdit->setReadOnly(true);
    m_facilityIdEdit->setPlaceholderText(tr("自动生成"));
    basicLayout->addRow(tr("设施编号:"), m_facilityIdEdit);
    
    m_facilityNameEdit = new QLineEdit(this);
    m_facilityNameEdit->setPlaceholderText(tr("请输入设施名称"));
    basicLayout->addRow(tr("设施名称*:"), m_facilityNameEdit);
    
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(tr("阀门"), "valve");
    m_typeCombo->addItem(tr("井盖"), "manhole");
    m_typeCombo->addItem(tr("泵站"), "pump_station");
    m_typeCombo->addItem(tr("调压站"), "pressure_station");
    m_typeCombo->addItem(tr("变压器"), "transformer");
    m_typeCombo->addItem(tr("接线盒"), "junction_box");
    basicLayout->addRow(tr("设施类型*:"), m_typeCombo);
    
    // 位置信息组
    auto *locationGroup = new QGroupBox(tr("位置信息"), this);
    auto *locationLayout = new QFormLayout(locationGroup);
    locationLayout->setSpacing(10);
    locationLayout->setContentsMargins(15, 15, 15, 15);
    
    m_longitudeSpin = new QDoubleSpinBox(this);
    m_longitudeSpin->setRange(-180, 180);
    m_longitudeSpin->setDecimals(6);
    m_longitudeSpin->setSuffix(tr(" °"));
    locationLayout->addRow(tr("经度*:"), m_longitudeSpin);
    
    m_latitudeSpin = new QDoubleSpinBox(this);
    m_latitudeSpin->setRange(-90, 90);
    m_latitudeSpin->setDecimals(6);
    m_latitudeSpin->setSuffix(tr(" °"));
    locationLayout->addRow(tr("纬度*:"), m_latitudeSpin);
    
    m_elevationSpin = new QDoubleSpinBox(this);
    m_elevationSpin->setRange(-1000, 10000);
    m_elevationSpin->setDecimals(2);
    m_elevationSpin->setSuffix(tr(" m"));
    locationLayout->addRow(tr("高程:"), m_elevationSpin);
    
    // 物理属性组
    auto *physicalGroup = new QGroupBox(tr("物理属性"), this);
    auto *physicalLayout = new QFormLayout(physicalGroup);
    physicalLayout->setSpacing(10);
    physicalLayout->setContentsMargins(15, 15, 15, 15);
    
    m_specEdit = new QLineEdit(this);
    m_specEdit->setPlaceholderText(tr("例如: DN200"));
    physicalLayout->addRow(tr("规格:"), m_specEdit);
    
    m_materialCombo = new QComboBox(this);
    m_materialCombo->addItems({tr("铸铁"), tr("球墨铸铁"), tr("钢"), tr("PE"), tr("PVC"), tr("混凝土"), tr("其他")});
    physicalLayout->addRow(tr("材质:"), m_materialCombo);
    
    // 关联信息组
    auto *relationGroup = new QGroupBox(tr("关联信息"), this);
    auto *relationLayout = new QFormLayout(relationGroup);
    relationLayout->setSpacing(10);
    relationLayout->setContentsMargins(15, 15, 15, 15);
    
    m_pipelineIdEdit = new QLineEdit(this);
    m_pipelineIdEdit->setPlaceholderText(tr("关联的管线编号"));
    relationLayout->addRow(tr("关联管线:"), m_pipelineIdEdit);
    
    // 建设信息组
    auto *constructionGroup = new QGroupBox(tr("建设信息"), this);
    auto *constructionLayout = new QFormLayout(constructionGroup);
    constructionLayout->setSpacing(10);
    constructionLayout->setContentsMargins(15, 15, 15, 15);
    
    m_buildDateEdit = new QDateEdit(this);
    m_buildDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_buildDateEdit->setCalendarPopup(true);
    constructionLayout->addRow(tr("建设日期:"), m_buildDateEdit);
    
    m_builderEdit = new QLineEdit(this);
    m_builderEdit->setPlaceholderText(tr("建设单位"));
    constructionLayout->addRow(tr("建设单位:"), m_builderEdit);
    
    m_ownerEdit = new QLineEdit(this);
    m_ownerEdit->setPlaceholderText(tr("产权单位"));
    constructionLayout->addRow(tr("产权单位:"), m_ownerEdit);
    
    // 运维信息组
    auto *maintenanceGroup = new QGroupBox(tr("运维信息"), this);
    auto *maintenanceLayout = new QFormLayout(maintenanceGroup);
    maintenanceLayout->setSpacing(10);
    maintenanceLayout->setContentsMargins(15, 15, 15, 15);
    
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem(tr("运行中"), "active");
    m_statusCombo->addItem(tr("停用"), "inactive");
    m_statusCombo->addItem(tr("维修中"), "repairing");
    m_statusCombo->addItem(tr("废弃"), "abandoned");
    maintenanceLayout->addRow(tr("运行状态:"), m_statusCombo);
    
    m_healthScoreSpin = new QSpinBox(this);
    m_healthScoreSpin->setRange(0, 100);
    m_healthScoreSpin->setSuffix(tr(" 分"));
    m_healthScoreSpin->setValue(100);
    maintenanceLayout->addRow(tr("健康度:"), m_healthScoreSpin);
    
    m_lastInspectionEdit = new QDateEdit(this);
    m_lastInspectionEdit->setDisplayFormat("yyyy-MM-dd");
    m_lastInspectionEdit->setCalendarPopup(true);
    maintenanceLayout->addRow(tr("上次巡检:"), m_lastInspectionEdit);
    
    m_maintenanceUnitEdit = new QLineEdit(this);
    m_maintenanceUnitEdit->setPlaceholderText(tr("养护单位"));
    maintenanceLayout->addRow(tr("养护单位:"), m_maintenanceUnitEdit);
    
    // 备注组
    auto *remarksGroup = new QGroupBox(tr("备注"), this);
    auto *remarksLayout = new QVBoxLayout(remarksGroup);
    remarksLayout->setContentsMargins(15, 15, 15, 15);
    
    m_remarksEdit = new QTextEdit(this);
    m_remarksEdit->setMinimumHeight(80);
    m_remarksEdit->setPlaceholderText(tr("备注信息"));
    remarksLayout->addWidget(m_remarksEdit);
    
    // 将所有组添加到滚动布局
    scrollLayout->addWidget(basicGroup);
    scrollLayout->addWidget(locationGroup);
    scrollLayout->addWidget(physicalGroup);
    scrollLayout->addWidget(relationGroup);
    scrollLayout->addWidget(constructionGroup);
    scrollLayout->addWidget(maintenanceGroup);
    scrollLayout->addWidget(remarksGroup);
    scrollLayout->addStretch(); // 添加弹性空间
    
    // 设置滚动区域的内容
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    
    // 按钮布局（固定在底部，不滚动）
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton(tr("保存"), this);
    m_saveBtn->setMinimumWidth(100);
    m_cancelBtn = new QPushButton(tr("取消"), this);
    m_cancelBtn->setMinimumWidth(100);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_cancelBtn);
    
    // 添加到主布局
    mainLayout->addWidget(scrollArea, 1); // 1表示拉伸因子，让滚动区域占据剩余空间
    mainLayout->addLayout(btnLayout);
    
    // 连接信号
    connect(m_saveBtn, &QPushButton::clicked, this, &FacilityEditDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &FacilityEditDialog::onCancel);
}

void FacilityEditDialog::populateFields(const Facility &facility)
{
    if (facility.isValid()) {
        m_facilityIdEdit->setText(facility.facilityId());
    } else {
        // 新建设施时，如果编号为空则自动生成
        if (facility.facilityId().isEmpty() && !facility.facilityType().isEmpty()) {
            QString autoId = IdGenerator::generateFacilityId(facility.facilityType());
            m_facilityIdEdit->setText(autoId);
        }
    }
    
    m_facilityNameEdit->setText(facility.facilityName());
    
    // 设置类型
    int typeIdx = m_typeCombo->findData(facility.facilityType());
    if (typeIdx >= 0) {
        m_typeCombo->setCurrentIndex(typeIdx);
    }
    
    // 位置信息
    QPointF coord = facility.coordinate();
    if (coord.x() != 0.0 || coord.y() != 0.0) {
        m_longitudeSpin->setValue(coord.x());
        m_latitudeSpin->setValue(coord.y());
    }
    m_elevationSpin->setValue(facility.elevationM());
    
    // 物理属性
    m_specEdit->setText(facility.spec());
    
    int materialIdx = m_materialCombo->findText(facility.material());
    if (materialIdx >= 0) {
        m_materialCombo->setCurrentIndex(materialIdx);
    }
    
    // 关联信息
    m_pipelineIdEdit->setText(facility.pipelineId());
    
    // 建设信息
    if (facility.buildDate().isValid()) {
        m_buildDateEdit->setDate(facility.buildDate());
    }
    m_builderEdit->setText(facility.builder());
    m_ownerEdit->setText(facility.owner());
    
    // 运维信息
    int statusIdx = m_statusCombo->findData(facility.status());
    if (statusIdx >= 0) {
        m_statusCombo->setCurrentIndex(statusIdx);
    }
    m_healthScoreSpin->setValue(facility.healthScore());
    
    if (facility.lastMaintenance().isValid()) {
        m_lastInspectionEdit->setDate(facility.lastMaintenance());
    }
    m_maintenanceUnitEdit->setText(facility.maintenanceUnit());
    
    // 备注
    m_remarksEdit->setPlainText(facility.remarks());
}

Facility FacilityEditDialog::collectInput() const
{
    Facility f = m_facility;
    
    f.setFacilityId(m_facilityIdEdit->text().trimmed());
    f.setFacilityName(m_facilityNameEdit->text().trimmed());
    f.setFacilityType(m_typeCombo->currentData().toString());
    
    f.setCoordinate(QPointF(m_longitudeSpin->value(), m_latitudeSpin->value()));
    f.setElevationM(m_elevationSpin->value());
    
    f.setSpec(m_specEdit->text().trimmed());
    f.setMaterial(m_materialCombo->currentText());
    
    f.setPipelineId(m_pipelineIdEdit->text().trimmed());
    
    if (m_buildDateEdit->date().isValid() && m_buildDateEdit->date().year() > 2000) {
        f.setBuildDate(m_buildDateEdit->date());
    }
    f.setBuilder(m_builderEdit->text().trimmed());
    f.setOwner(m_ownerEdit->text().trimmed());
    
    f.setStatus(m_statusCombo->currentData().toString());
    f.setHealthScore(m_healthScoreSpin->value());
    
    if (m_lastInspectionEdit->date().isValid() && m_lastInspectionEdit->date().year() > 2000) {
        f.setLastMaintenance(m_lastInspectionEdit->date());
    }
    f.setMaintenanceUnit(m_maintenanceUnitEdit->text().trimmed());
    
    f.setRemarks(m_remarksEdit->toPlainText().trimmed());
    
    return f;
}

QString FacilityEditDialog::getTypeDisplayName(const QString &type) const
{
    if (type == "valve") return tr("阀门");
    if (type == "manhole") return tr("井盖");
    if (type == "pump_station") return tr("泵站");
    if (type == "pressure_station") return tr("调压站");
    if (type == "transformer") return tr("变压器");
    if (type == "junction_box") return tr("接线盒");
    return type;
}

void FacilityEditDialog::onSave()
{
    // 验证必填字段
    if (m_facilityNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请填写设施名称"));
        m_facilityNameEdit->setFocus();
        return;
    }
    
    if (m_typeCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请选择设施类型"));
        return;
    }
    
    if (m_longitudeSpin->value() == 0.0 && m_latitudeSpin->value() == 0.0) {
        QMessageBox::warning(this, tr("提示"), tr("请设置设施位置（经度和纬度）"));
        return;
    }
    
    m_facility = collectInput();
    accept();
}

void FacilityEditDialog::onCancel()
{
    reject();
}

