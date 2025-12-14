#include "customcheckbox.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QStyle>

CustomCheckBox::CustomCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    // 设置样式，与登录界面风格一致
    // spacing 控制复选框和文字之间的距离
    setStyleSheet(
        "CustomCheckBox {"
        "    color: #34495e;"
        "    font-size: 12px;"
        "    spacing: 18px;"
        "}"
        "CustomCheckBox::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "}"
    );
}

QSize CustomCheckBox::sizeHint() const
{
    QSize size = QCheckBox::sizeHint();
    // 确保indicator有足够空间
    return size;
}

CustomCheckBox::~CustomCheckBox()
{
}

void CustomCheckBox::paintEvent(QPaintEvent *event)
{
    QStyleOptionButton option;
    initStyleOption(&option);
    
    QStylePainter painter(this);
    
    // 获取indicator的位置
    QRect indicatorRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
    
    // 绘制自定义indicator（先绘制复选框）
    drawIndicator(painter, indicatorRect, isChecked());
    
    // 获取文本区域
    QRect textRect = style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this);
    
    // 手动调整文本位置，在indicator右侧添加额外间距
    // 计算indicator的实际右边界（包括边框）
    int indicatorRight = indicatorRect.right();
    int spacing = 8; // 增加间距到8px，确保文字不会重叠
    textRect.setLeft(indicatorRight + spacing);
    
    // 绘制文本，使用调整后的位置
    option.rect = textRect;
    painter.drawControl(QStyle::CE_CheckBoxLabel, option);
}

void CustomCheckBox::drawIndicator(QPainter &painter, const QRect &rect, bool checked)
{
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 确保是正方形，取较小的边作为尺寸
    int size = qMin(rect.width(), rect.height());
    int x = rect.x() + (rect.width() - size) / 2;
    int y = rect.y() + (rect.height() - size) / 2;
    QRect boxRect(x, y, size, size);
    
    // 稍微缩小以避免边框被裁剪
    boxRect = boxRect.adjusted(1, 1, -1, -1);
    size = boxRect.width(); // 更新size
    
    QPen borderPen(QColor("#e0e0e0"), 2);
    QBrush bgBrush(QColor("#ffffff"));
    
    // 如果选中，边框变为蓝色，与输入框聚焦状态一致
    if (checked) {
        borderPen.setColor(QColor("#3498db"));
    }
    
    painter.setPen(borderPen);
    painter.setBrush(bgBrush);
    // 使用圆角，圆角半径约为边长的1/4，使其看起来更圆润
    int radius = size / 4;
    painter.drawRoundedRect(boxRect, radius, radius);
    
    // 如果选中，绘制勾选标记
    if (checked) {
        QPen checkPen(QColor("#3498db"), 2.2);
        checkPen.setCapStyle(Qt::RoundCap);
        checkPen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(checkPen);
        
        QPainterPath checkPath;
        int boxX = boxRect.x();
        int boxY = boxRect.y();
        int w = boxRect.width();
        int h = boxRect.height();
        
        // 计算勾选标记的位置（居中，更精细的调整）
        // 勾选标记从左侧1/4处开始，到中间偏下，再到右侧1/4处
        checkPath.moveTo(boxX + w * 0.25, boxY + h * 0.5);
        checkPath.lineTo(boxX + w * 0.45, boxY + h * 0.7);
        checkPath.lineTo(boxX + w * 0.75, boxY + h * 0.3);
        
        painter.drawPath(checkPath);
    }
}

