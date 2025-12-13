#include "customcombobox.h"
#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QStyle>
#include <QPolygonF>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void CustomComboBox::paintEvent(QPaintEvent *event)
{
    QComboBox::paintEvent(event);
    
    // 绘制自定义倒三角箭头
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    
    QRect rect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
    
    if (rect.isValid()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#95a5a6"));
        
        // 绘制倒等边三角形（顶点朝下）
        QPolygonF triangle;
        int centerX = rect.center().x();
        int centerY = rect.center().y();
        int size = 5; // 等边三角形的边长的一半（从中心到顶点的距离）
        
        // 等边三角形：高度 = 边长 * sqrt(3) / 2
        // 对于倒三角形，顶点在下，底边在上
        double height = size * 1.732; // sqrt(3) ≈ 1.732
        
        triangle << QPointF(centerX, centerY + height / 2)  // 下顶点（倒三角的尖）
                 << QPointF(centerX - size, centerY - height / 2)  // 左上
                 << QPointF(centerX + size, centerY - height / 2);  // 右上
        
        painter.drawPolygon(triangle);
    }
}

