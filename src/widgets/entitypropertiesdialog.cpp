#include "widgets/entitypropertiesdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QMessageBox>

EntityPropertiesDialog::EntityPropertiesDialog(QGraphicsItem *item, 
                                             EntityType type,
                                             QWidget *parent)
    : QDialog(parent)
    , m_item(item)
    , m_entityType(type)
{
    setupUI();
    loadProperties();
}

EntityPropertiesDialog::~EntityPropertiesDialog()
{
}

void EntityPropertiesDialog::setupUI()
{
    setWindowTitle(m_entityType == Pipeline ? "管线属性" : "设施属性");
    setMinimumWidth(400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    
    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("📋 基本信息", this);
    QFormLayout *basicLayout = new QFormLayout(basicGroup);
    basicLayout->setSpacing(8);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("请输入名称");
    basicLayout->addRow("名称:", m_nameEdit);
    
    m_typeCombo = new QComboBox(this);
    if (m_entityType == Pipeline) {
        m_typeCombo->addItem("💧 给水管", "water_supply");
        m_typeCombo->addItem("🚰 排水管", "sewage");
        m_typeCombo->addItem("🔥 燃气管", "gas");
        m_typeCombo->addItem("⚡ 电力电缆", "electric");
        m_typeCombo->addItem("📡 通信光缆", "telecom");
        m_typeCombo->addItem("🌡️ 供热管", "heat");
    } else {
        m_typeCombo->addItem("🔵 阀门", "valve");
        m_typeCombo->addItem("🟢 井盖", "manhole");
        m_typeCombo->addItem("🏗️ 泵站", "pump_station");
        m_typeCombo->addItem("🔌 变压器", "transformer");
        m_typeCombo->addItem("⚙️ 调压站", "regulator");
        m_typeCombo->addItem("📦 接线盒", "junction_box");
    }
    basicLayout->addRow("类型:", m_typeCombo);
    
    m_descEdit = new QTextEdit(this);
    m_descEdit->setPlaceholderText("请输入描述信息");
    m_descEdit->setMaximumHeight(80);
    basicLayout->addRow("描述:", m_descEdit);
    
    mainLayout->addWidget(basicGroup);
    
    // 样式设置组
    QGroupBox *styleGroup = new QGroupBox("🎨 样式设置", this);
    QFormLayout *styleLayout = new QFormLayout(styleGroup);
    styleLayout->setSpacing(8);
    
    m_colorCombo = new QComboBox(this);
    m_colorCombo->addItem("🔵 蓝色", "#1890ff");
    m_colorCombo->addItem("🔴 红色", "#ff4d4f");
    m_colorCombo->addItem("🟢 绿色", "#52c41a");
    m_colorCombo->addItem("🟡 黄色", "#faad14");
    m_colorCombo->addItem("🟣 紫色", "#722ed1");
    m_colorCombo->addItem("🟠 橙色", "#fa8c16");
    m_colorCombo->addItem("⚫ 灰色", "#8c8c8c");
    styleLayout->addRow("颜色:", m_colorCombo);
    
    m_lineWidthSpin = new QSpinBox(this);
    m_lineWidthSpin->setRange(1, 10);
    m_lineWidthSpin->setValue(3);
    m_lineWidthSpin->setSuffix(" px");
    if (m_entityType == Pipeline) {
        styleLayout->addRow("线宽:", m_lineWidthSpin);
    }
    
    mainLayout->addWidget(styleGroup);
    
    // 按钮组
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);
    
    m_saveBtn = new QPushButton("💾 保存", this);
    m_saveBtn->setStyleSheet(
        "QPushButton { background-color: #1890ff; color: white; border: none; "
        "border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #40a9ff; }"
    );
    
    m_deleteBtn = new QPushButton("🗑️ 删除", this);
    m_deleteBtn->setStyleSheet(
        "QPushButton { background-color: #ff4d4f; color: white; border: none; "
        "border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #ff7875; }"
    );
    
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setStyleSheet(
        "QPushButton { background-color: #f0f0f0; color: #333; border: 1px solid #d9d9d9; "
        "border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #e6e6e6; }"
    );
    
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    
    mainLayout->addLayout(btnLayout);
    
    // 连接信号
    connect(m_saveBtn, &QPushButton::clicked, this, &EntityPropertiesDialog::onSaveClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &EntityPropertiesDialog::onDeleteClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void EntityPropertiesDialog::loadProperties()
{
    if (!m_item) {
        return;
    }
    
    // 从QGraphicsItem的data中加载属性
    m_nameEdit->setText(m_item->data(0).toString());
    
    QString typeId = m_item->data(1).toString();
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toString() == typeId) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    m_descEdit->setPlainText(m_item->data(2).toString());
    
    // 加载颜色
    QColor color = m_item->data(3).value<QColor>();
    if (color.isValid()) {
        QString colorName = color.name();
        for (int i = 0; i < m_colorCombo->count(); ++i) {
            if (m_colorCombo->itemData(i).toString() == colorName) {
                m_colorCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    
    // 加载线宽
    int lineWidth = m_item->data(4).toInt();
    if (lineWidth > 0) {
        m_lineWidthSpin->setValue(lineWidth);
    }
}

void EntityPropertiesDialog::saveProperties()
{
    if (!m_item) {
        return;
    }
    
    // 保存到QGraphicsItem的data中
    m_item->setData(0, m_nameEdit->text());
    m_item->setData(1, m_typeCombo->currentData().toString());
    m_item->setData(2, m_descEdit->toPlainText());
    m_item->setData(3, QColor(m_colorCombo->currentData().toString()));
    m_item->setData(4, m_lineWidthSpin->value());
    
    // 更新图形项的显示
    QColor newColor = getColor();
    int newWidth = getLineWidth();
    
    if (auto pathItem = dynamic_cast<QGraphicsPathItem*>(m_item)) {
        QPen pen = pathItem->pen();
        pen.setColor(newColor);
        pen.setWidth(newWidth);
        pathItem->setPen(pen);
    } else if (auto ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(m_item)) {
        ellipseItem->setBrush(QBrush(newColor));
        QPen pen = ellipseItem->pen();
        pen.setColor(newColor.darker(120));
        ellipseItem->setPen(pen);
    }
    
    emit propertiesChanged();
}

void EntityPropertiesDialog::onSaveClicked()
{
    saveProperties();
    accept();
}

void EntityPropertiesDialog::onDeleteClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这个" + QString(m_entityType == Pipeline ? "管线" : "设施") + "吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit deleteRequested();
        accept();
    }
}

QString EntityPropertiesDialog::getName() const
{
    return m_nameEdit->text();
}

QString EntityPropertiesDialog::getType() const
{
    return m_typeCombo->currentData().toString();
}

QString EntityPropertiesDialog::getDescription() const
{
    return m_descEdit->toPlainText();
}

QColor EntityPropertiesDialog::getColor() const
{
    return QColor(m_colorCombo->currentData().toString());
}

int EntityPropertiesDialog::getLineWidth() const
{
    return m_lineWidthSpin->value();
}
