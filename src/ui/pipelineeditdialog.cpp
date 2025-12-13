#include "ui/pipelineeditdialog.h"
#include "core/utils/idgenerator.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QtMath>  // for qSqrt

PipelineEditDialog::PipelineEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_id(-1)
{
    setupUI();
    setupConnections();
    
    setWindowTitle("管线属性编辑");
    resize(500, 600);
}

PipelineEditDialog::~PipelineEditDialog()
{
}

void PipelineEditDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // ========== 基本信息组 ==========
    QGroupBox *basicGroup = new QGroupBox("📋 基本信息");
    QFormLayout *basicLayout = new QFormLayout();
    basicLayout->setSpacing(10);
    
    // ID字段（只读，灰色背景）
    m_idEdit = new QLineEdit();
    m_idEdit->setReadOnly(true);
    m_idEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #f5f5f5;"
        "  color: #666;"
        "  border: 1px solid #ddd;"
        "  padding: 5px;"
        "  border-radius: 3px;"
        "}"
    );
    m_idEdit->setPlaceholderText("自动生成");
    basicLayout->addRow("🆔 管线ID:", m_idEdit);
    
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("请输入管线名称");
    basicLayout->addRow("管线名称 *:", m_nameEdit);
    
    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("💧 给水管", "water_supply");
    m_typeCombo->addItem("🚰 排水管", "sewage");
    m_typeCombo->addItem("🔥 燃气管", "gas");
    m_typeCombo->addItem("⚡ 电力电缆", "electric");
    m_typeCombo->addItem("📡 通信光缆", "telecom");
    m_typeCombo->addItem("🌡️ 供热管", "heat");
    basicLayout->addRow("管线类型 *:", m_typeCombo);
    
    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("例如: WS-001");
    basicLayout->addRow("管线编号:", m_codeEdit);
    
    basicGroup->setLayout(basicLayout);
    mainLayout->addWidget(basicGroup);
    
    // ========== 物理属性组 ==========
    QGroupBox *physicalGroup = new QGroupBox("🔧 物理属性");
    QFormLayout *physicalLayout = new QFormLayout();
    physicalLayout->setSpacing(10);
    
    m_diameterSpin = new QDoubleSpinBox();
    m_diameterSpin->setRange(0, 5000);
    m_diameterSpin->setSuffix(" mm");
    m_diameterSpin->setValue(300);
    physicalLayout->addRow("管径:", m_diameterSpin);
    
    m_materialCombo = new QComboBox();
    m_materialCombo->addItems({"铸铁", "球墨铸铁", "钢管", "PE管", "PVC管", "混凝土", "其他"});
    physicalLayout->addRow("材质:", m_materialCombo);
    
    m_lengthSpin = new QDoubleSpinBox();
    m_lengthSpin->setRange(0, 100000);
    m_lengthSpin->setSuffix(" m");
    m_lengthSpin->setDecimals(2);
    m_lengthSpin->setValue(0);
    m_lengthSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "  background-color: #fffacd;"
        "  border: 1px solid #ffa500;"
        "}"
    );
    physicalLayout->addRow("长度 (自动计算):", m_lengthSpin);
    
    m_depthSpin = new QDoubleSpinBox();
    m_depthSpin->setRange(0, 50);
    m_depthSpin->setSuffix(" m");
    m_depthSpin->setDecimals(2);
    m_depthSpin->setValue(1.5);
    physicalLayout->addRow("埋深:", m_depthSpin);
    
    m_pressureCombo = new QComboBox();
    m_pressureCombo->addItems({"低压", "中压", "高压", "特高压"});
    physicalLayout->addRow("压力等级:", m_pressureCombo);
    
    physicalGroup->setLayout(physicalLayout);
    mainLayout->addWidget(physicalGroup);
    
    // ========== 建设信息组 ==========
    QGroupBox *constructionGroup = new QGroupBox("🏗️ 建设信息");
    QFormLayout *constructionLayout = new QFormLayout();
    constructionLayout->setSpacing(10);
    
    m_constructionDateEdit = new QDateEdit(QDate::currentDate());
    m_constructionDateEdit->setCalendarPopup(true);
    m_constructionDateEdit->setDisplayFormat("yyyy-MM-dd");
    constructionLayout->addRow("建设日期:", m_constructionDateEdit);
    
    m_constructorEdit = new QLineEdit();
    m_constructorEdit->setPlaceholderText("施工单位名称");
    constructionLayout->addRow("施工单位:", m_constructorEdit);
    
    m_statusCombo = new QComboBox();
    m_statusCombo->addItems({"正常", "维修中", "停用", "报废"});
    constructionLayout->addRow("运行状态:", m_statusCombo);
    
    constructionGroup->setLayout(constructionLayout);
    mainLayout->addWidget(constructionGroup);
    
    // ========== 备注 ==========
    QGroupBox *remarkGroup = new QGroupBox("📝 备注");
    QVBoxLayout *remarkLayout = new QVBoxLayout();
    
    m_remarkEdit = new QTextEdit();
    m_remarkEdit->setPlaceholderText("请输入备注信息...");
    m_remarkEdit->setMaximumHeight(80);
    remarkLayout->addWidget(m_remarkEdit);
    
    remarkGroup->setLayout(remarkLayout);
    mainLayout->addWidget(remarkGroup);
    
    mainLayout->addStretch();
    
    // ========== 按钮区域 ==========
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton *cancelBtn = new QPushButton("取消");
    QPushButton *okBtn = new QPushButton("确定");
    okBtn->setDefault(true);
    
    cancelBtn->setMinimumSize(100, 35);
    okBtn->setMinimumSize(100, 35);
    
    okBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF7A18;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #FF8C3A;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #E66A08;"
        "}"
    );
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(okBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void PipelineEditDialog::setupConnections()
{
    connect(this, &QDialog::accepted, this, &PipelineEditDialog::onAccepted);
    connect(this, &QDialog::rejected, this, &PipelineEditDialog::onRejected);
}

void PipelineEditDialog::setPipelineType(const QString &type)
{
    // 根据类型ID查找并设置下拉框
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toString() == type) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }
}

void PipelineEditDialog::setGeometry(const QString &wkt)
{
    m_wktGeometry = wkt;
    qDebug() << "Set geometry WKT:" << wkt;
}

void PipelineEditDialog::setAutoGeneratedCode(int id, const QString &type)
{
    Q_UNUSED(id);  // 不再使用id参数，改用日期+序号格式
    
    // 使用IdGenerator生成编号（基础编号，保存时会检查唯一性）
    QString code = IdGenerator::generatePipelineId(type);
    m_codeEdit->setText(code);
    
    qDebug() << "✨ Auto-generated pipeline code:" << code << "for type:" << type;
}

void PipelineEditDialog::setPipelineId(const QString &id)
{
    if (m_codeEdit) {
        m_codeEdit->setText(id);
    }
}

void PipelineEditDialog::calculateAndSetLength(const QVector<QPointF> &points)
{
    if (points.size() < 2) {
        m_lengthSpin->setValue(0);
        return;
    }
    
    // 计算总长度（场景坐标距离）
    // 注意：这只是像素距离，不是实际距离
    // 实际距离应使用 calculateAndSetLengthFromWKT 基于地理坐标计算
    double totalLength = 0.0;
    for (int i = 1; i < points.size(); ++i) {
        QPointF p1 = points[i - 1];
        QPointF p2 = points[i];
        
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double segmentLength = qSqrt(dx * dx + dy * dy);
        
        totalLength += segmentLength;
    }
    
    // 像素距离转换（粗略估计）
    // 这里不准确，应该使用 WKT 计算
    m_lengthSpin->setValue(totalLength);
    
    qDebug() << "⚠️ Scene coordinate length (pixels):" << totalLength << "(" << points.size() << "points)";
    qDebug() << "⚠️ This is NOT accurate! Use calculateAndSetLengthFromWKT for real distance.";
}

void PipelineEditDialog::calculateAndSetLengthFromWKT(const QString &wkt)
{
    if (wkt.isEmpty() || !wkt.startsWith("LINESTRING")) {
        m_lengthSpin->setValue(0);
        return;
    }
    
    // 解析 WKT 格式: LINESTRING(lon1 lat1, lon2 lat2, ...)
    QString coordsStr = wkt;
    coordsStr.remove("LINESTRING(");
    coordsStr.remove(")");
    
    QStringList pointStrs = coordsStr.split(",", Qt::SkipEmptyParts);
    if (pointStrs.size() < 2) {
        m_lengthSpin->setValue(0);
        return;
    }
    
    // 解析坐标点
    QVector<QPointF> geoPoints;
    for (const QString &pointStr : pointStrs) {
        QStringList coords = pointStr.trimmed().split(" ", Qt::SkipEmptyParts);
        if (coords.size() == 2) {
            double lon = coords[0].toDouble();
            double lat = coords[1].toDouble();
            geoPoints.append(QPointF(lon, lat));
        }
    }
    
    if (geoPoints.size() < 2) {
        m_lengthSpin->setValue(0);
        return;
    }
    
    // 使用 Haversine 公式计算地球表面两点间的实际距离
    double totalLength = 0.0;
    const double EARTH_RADIUS = 6371000.0;  // 地球半径（米）
    
    for (int i = 1; i < geoPoints.size(); ++i) {
        QPointF p1 = geoPoints[i - 1];
        QPointF p2 = geoPoints[i];
        
        // 转换为弧度
        double lat1 = qDegreesToRadians(p1.y());
        double lon1 = qDegreesToRadians(p1.x());
        double lat2 = qDegreesToRadians(p2.y());
        double lon2 = qDegreesToRadians(p2.x());
        
        // Haversine 公式
        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;
        
        double a = qSin(dLat / 2) * qSin(dLat / 2) +
                   qCos(lat1) * qCos(lat2) *
                   qSin(dLon / 2) * qSin(dLon / 2);
        
        double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
        double distance = EARTH_RADIUS * c;
        
        totalLength += distance;
    }
    
    // 设置长度（米）
    m_lengthSpin->setValue(totalLength);
    
    qDebug() << "📏 Calculated pipeline length (Haversine):" << totalLength << "m (" << geoPoints.size() << "points)";
}

Pipeline PipelineEditDialog::getPipeline() const
{
    Pipeline pipeline;
    
    pipeline.setId(m_id);
    pipeline.setPipelineName(m_nameEdit->text().trimmed());
    pipeline.setPipelineType(m_typeCombo->currentData().toString());
    pipeline.setPipelineId(m_codeEdit->text().trimmed());
    pipeline.setGeomWkt(m_wktGeometry);
    
    pipeline.setDiameterMm(m_diameterSpin->value());
    pipeline.setMaterial(m_materialCombo->currentText());
    pipeline.setLengthM(m_lengthSpin->value());
    pipeline.setDepthM(m_depthSpin->value());
    pipeline.setPressureClass(m_pressureCombo->currentText());
    
    pipeline.setBuildDate(m_constructionDateEdit->date());
    pipeline.setBuilder(m_constructorEdit->text().trimmed());
    pipeline.setStatus(m_statusCombo->currentText());
    
    pipeline.setRemarks(m_remarkEdit->toPlainText().trimmed());
    
    return pipeline;
}

void PipelineEditDialog::loadPipeline(const Pipeline &pipeline)
{
    m_id = pipeline.id();
    
    // 显示ID
    m_idEdit->setText(QString::number(m_id));
    
    m_nameEdit->setText(pipeline.pipelineName());
    setPipelineType(pipeline.pipelineType());
    m_codeEdit->setText(pipeline.pipelineId());
    m_wktGeometry = pipeline.geomWkt();
    
    m_diameterSpin->setValue(pipeline.diameterMm());
    
    int materialIndex = m_materialCombo->findText(pipeline.material());
    if (materialIndex >= 0) {
        m_materialCombo->setCurrentIndex(materialIndex);
    }
    
    m_lengthSpin->setValue(pipeline.lengthM());
    m_depthSpin->setValue(pipeline.depthM());
    
    int pressureIndex = m_pressureCombo->findText(pipeline.pressureClass());
    if (pressureIndex >= 0) {
        m_pressureCombo->setCurrentIndex(pressureIndex);
    }
    
    m_constructionDateEdit->setDate(pipeline.buildDate());
    m_constructorEdit->setText(pipeline.builder());
    
    int statusIndex = m_statusCombo->findText(pipeline.status());
    if (statusIndex >= 0) {
        m_statusCombo->setCurrentIndex(statusIndex);
    }
    
    m_remarkEdit->setPlainText(pipeline.remarks());
}

void PipelineEditDialog::onAccepted()
{
    // 验证必填字段
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入管线名称！");
        m_nameEdit->setFocus();
        return;
    }
    
    qDebug() << "Pipeline edit dialog accepted";
}

void PipelineEditDialog::onRejected()
{
    qDebug() << "Pipeline edit dialog rejected";
}

QString PipelineEditDialog::getTypeDisplayName(const QString &typeId) const
{
    static QMap<QString, QString> typeNames = {
        {"water_supply", "给水管"},
        {"sewage", "排水管"},
        {"gas", "燃气管"},
        {"electric", "电力电缆"},
        {"telecom", "通信光缆"},
        {"heat", "供热管"}
    };
    return typeNames.value(typeId, "未知类型");
}
