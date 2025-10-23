#include "FloatMessage.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

FloatMessage::FloatMessage(const QString &text, QWidget *parent, int duration)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    m_label = new QLabel(text, this);
    m_label->setStyleSheet(R"(
        QLabel {
            background-color: rgba(46, 204, 113, 220); /* 绿色背景 */
            color: white;
            padding: 12px 18px;
            border-radius: 6px;
            font-size: 14px;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_label);
    setLayout(layout);

    // 初始设置完全透明
    setWindowOpacity(0.0);

    show(); // 先显示（虽然是透明的）

    // 1. 创建淡入动画
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
    fadeIn->setDuration(500); // 0.5秒
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    // 2. 创建滑入动画
    QPropertyAnimation *slideIn = new QPropertyAnimation(this, "geometry");
    slideIn->setDuration(500);
    slideIn->setStartValue(geometry());

    // 3. 创建淡出动画
    QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    // 连接动画信号
    connect(fadeIn, &QPropertyAnimation::finished,
            [this, fadeOut, duration]()
            {
                // 淡入完成后，等待指定时间，然后开始淡出
                QTimer::singleShot(
                    duration, fadeOut,
                    [fadeOut]()
                    {
                        fadeOut->start(); // 在 lambda
                                          // 内部，调用无参版本，使用默认参数
                    });
            });

    connect(fadeOut, &QPropertyAnimation::finished, this, &QWidget::close);

    // 启动淡入动画
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

FloatMessage::~FloatMessage() {}

void FloatMessage::closeEvent(QCloseEvent *event)
{
    emit closed();
    event->accept();
}

void FloatMessage::setPosition(int x, int y)
{
    move(x - width(), y);
    // move(0, 0);
}
