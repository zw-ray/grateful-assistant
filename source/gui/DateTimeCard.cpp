#include "DateTimeCard.h"
#include <QBrush>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QFontDatabase>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QStyleOption>
#include <QStylePainter>
#include <QTimer>
#include <QVBoxLayout>

DateTimeCard::DateTimeCard(QWidget *parent) : Card(parent)
{
    initUI();
    setAutoFillBackground(true);
}

DateTimeCard::~DateTimeCard() {}

void DateTimeCard::initUI()
{
    // 时间标签
    QLabel *timeLabel = new QLabel(this);
    timeLabel->setObjectName("timeLabel");

    // 日期标签
    QLabel *dateLabel = new QLabel(this);
    dateLabel->setObjectName("dateLabel");

    // 星期标签
    QLabel *weekdayLabel = new QLabel(this);
    weekdayLabel->setObjectName("weekdayLabel");

    // 定时器设置
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &DateTimeCard::updateTime);
    connect(updateTimer, &QTimer::timeout, this, &DateTimeCard::updateDate);
    updateTimer->setInterval(1000);
    updateTimer->start();

    // 容器布局
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->addStretch();
    containerLayout->addWidget(timeLabel, 0, Qt::AlignCenter);
    containerLayout->addWidget(dateLabel, 0, Qt::AlignCenter);
    containerLayout->addWidget(weekdayLabel, 0, Qt::AlignCenter);
    containerLayout->addStretch();
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(5);

    // 初始更新
    updateTime();
    updateDate();
}

void DateTimeCard::updateTime()
{
    QLabel *timeLabel = findChild<QLabel *>("timeLabel");
    timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    update();
}

void DateTimeCard::updateDate()
{
    QDateTime now = QDateTime::currentDateTime();

    QLabel *dateLabel = findChild<QLabel *>("dateLabel");
    dateLabel->setText(now.toString("yyyy年MM月dd日"));

    QLabel *weekdayLabel = findChild<QLabel *>("weekdayLabel");
    const QStringList weekdays = {"星期一", "星期二", "星期三", "星期四",
                                  "星期五", "星期六", "星期日"};
    weekdayLabel->setText(weekdays[now.date().dayOfWeek() - 1]);
}

void DateTimeCard::paintEvent(QPaintEvent *event)
{
    // 调用基类的paintEvent以确保卡片基础样式正确绘制
    Card::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置半透明背景 - 使用圆角矩形以保留卡片原有圆角效果
    QBrush backgroundBrush(QColor(255, 255, 255, 180));
    painter.setBrush(backgroundBrush);
    painter.setPen(Qt::NoPen);

    // 创建圆角矩形路径
    QPainterPath path;
    int borderRadius = 10; // 根据卡片实际圆角大小调整
    path.addRoundedRect(rect(), borderRadius, borderRadius);
    painter.drawPath(path);

    // 获取当前时间
    QTime currentTime = QTime::currentTime();
    int hours = currentTime.hour();
    int minutes = currentTime.minute();
    int seconds = currentTime.second();

    // 绘制时钟的中心点
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(centerX, centerY); // 以卡片最小边的一半作为半径

    // 绘制时钟刻度到卡片边缘

    // 绘制12个主要刻度（小时刻度）
    QPen majorTickPen(QColor(100, 100, 150, 150), 2);
    painter.setPen(majorTickPen);

    for (int i = 0; i < 12; ++i)
    {
        double angle = 2 * M_PI * i / 12 - M_PI_2; // 从12点钟方向开始

        // 刻度线从边缘向内部延伸
        int startX = centerX + qCos(angle) * radius;
        int startY = centerY + qSin(angle) * radius;
        int endX = centerX + qCos(angle) * (radius - 25);
        int endY = centerY + qSin(angle) * (radius - 25);

        painter.drawLine(startX, startY, endX, endY);
    }

    // 绘制60个次要刻度（分钟刻度）
    QPen minorTickPen(QColor(150, 150, 200, 100), 1);
    painter.setPen(minorTickPen);

    for (int i = 0; i < 60; ++i)
    {
        if (i % 5 == 0)
        {
            continue; // 跳过已经绘制的小时刻度
        }

        double angle = 2 * M_PI * i / 60 - M_PI_2;

        // 刻度线从边缘向内部延伸
        int startX = centerX + qCos(angle) * radius;
        int startY = centerY + qSin(angle) * radius;
        int endX = centerX + qCos(angle) * (radius - 15);
        int endY = centerY + qSin(angle) * (radius - 15);

        painter.drawLine(startX, startY, endX, endY);
    }

    // 绘制时针
    double hourAngle = 2 * M_PI * (hours % 12 + minutes / 60.0) / 12 - M_PI_2;
    int hourHandLength = radius * 0.5;
    int hourX = centerX + qCos(hourAngle) * hourHandLength;
    int hourY = centerY + qSin(hourAngle) * hourHandLength;

    QPen hourPen(QColor(100, 100, 150, 200), 4);
    painter.setPen(hourPen);
    painter.drawLine(centerX, centerY, hourX, hourY);

    // 绘制分针
    double minuteAngle = 2 * M_PI * minutes / 60 - M_PI_2;
    int minuteHandLength = radius * 0.7;
    int minuteX = centerX + qCos(minuteAngle) * minuteHandLength;
    int minuteY = centerY + qSin(minuteAngle) * minuteHandLength;

    QPen minutePen(QColor(120, 120, 170, 200), 3);
    painter.setPen(minutePen);
    painter.drawLine(centerX, centerY, minuteX, minuteY);

    // 绘制秒针
    double secondAngle = 2 * M_PI * seconds / 60 - M_PI_2;
    int secondHandLength = radius * 0.8;
    int secondX = centerX + qCos(secondAngle) * secondHandLength;
    int secondY = centerY + qSin(secondAngle) * secondHandLength;

    QPen secondPen(QColor(255, 100, 100, 200), 2);
    painter.setPen(secondPen);
    painter.drawLine(centerX, centerY, secondX, secondY);

    // 绘制中心点
    painter.setBrush(QBrush(QColor(100, 100, 150, 200)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(centerX - 4, centerY - 4, 8, 8);

    // 绘制中心点装饰
    painter.setBrush(QBrush(QColor(255, 100, 100, 200)));
    painter.drawEllipse(centerX - 2, centerY - 2, 4, 4);
}
