#include "Card.h"
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEffect>

Card::Card(QWidget *parent) : QFrame(parent)
{
    setProperty("class", "Card");
    // 设置背景填充
    setAutoFillBackground(true);

    // 创建阴影效果
    QGraphicsDropShadowEffect *shadowEffect =
        new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(12); // 阴影模糊半径
    shadowEffect->setColor(
        QColor(0, 0, 0, 60));      // 阴影颜色和透明度（更淡的阴影）
    shadowEffect->setOffset(0, 0); // 阴影偏移为0，使阴影均匀分布在四周

    // 应用阴影效果
    setGraphicsEffect(shadowEffect);

    // 设置框架样式
    setFrameShadow(QFrame::Raised);
    setFrameShape(QFrame::Box);
    setLineWidth(1);
}

void Card::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    // QStyleOption opt;
    // opt.initFrom(this);
    // QPainter p(this);
    // style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
}

// 鼠标移入事件
void Card::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    // 增强阴影效果
    if (QGraphicsDropShadowEffect *shadowEffect =
            qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect()))
    {
        shadowEffect->setBlurRadius(16);             // 增加模糊半径
        shadowEffect->setColor(QColor(0, 0, 0, 80)); // 增加不透明度
        // 添加轻微的上浮效果
        this->setStyleSheet("margin-bottom: 2px; margin-right: 2px;");
    }
}

// 鼠标移出事件
void Card::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    // 恢复原始阴影效果
    if (QGraphicsDropShadowEffect *shadowEffect =
            qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect()))
    {
        shadowEffect->setBlurRadius(12);             // 恢复模糊半径
        shadowEffect->setColor(QColor(0, 0, 0, 60)); // 恢复不透明度
        // 移除上浮效果
        this->setStyleSheet("");
    }
}
