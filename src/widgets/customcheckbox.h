#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QCheckBox>

/**
 * @brief 自定义复选框
 * 支持自定义样式的勾选标记
 */
class CustomCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit CustomCheckBox(const QString &text, QWidget *parent = nullptr);
    ~CustomCheckBox();
    
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    void drawIndicator(QPainter &painter, const QRect &rect, bool checked);
};

#endif // CUSTOMCHECKBOX_H

