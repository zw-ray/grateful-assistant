#include "ListenContent.h"
#include <QHideEvent>
#include <QRandomGenerator>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QPainter>

ListenContent::ListenContent(QWidget *parent) : QWidget(parent), m_waveCount(WAVE_BAR_COUNT)
{
    initUI();
    initWaveform();
}

ListenContent::~ListenContent()
{
    if (m_animationTimer)
    {
        m_animationTimer->stop();
        delete m_animationTimer;
    }
}

void ListenContent::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 设置整体布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignCenter);

    // 聆听中文字提示
    m_listenLabel = new QLabel(tr("正在聆听..."), this);
    m_listenLabel->setAlignment(Qt::AlignCenter);
    m_listenLabel->setStyleSheet("font-size: 24px; color: #3973b6; font-weight: bold;");

    // 波形容器
    QWidget *waveContainer = new QWidget(this);
    waveContainer->setFixedSize(300, 100);
    waveContainer->setAttribute(Qt::WA_TranslucentBackground);

    m_waveLayout = new QHBoxLayout(waveContainer);
    m_waveLayout->setContentsMargins(0, 0, 0, 0);
    m_waveLayout->setSpacing(8);
    m_waveLayout->setAlignment(Qt::AlignCenter);
    m_waveCount = 300 / (8 + 6);

    // 添加到主布局
    mainLayout->addSpacing(0);
    mainLayout->addWidget(m_listenLabel);
    mainLayout->addWidget(waveContainer);
    mainLayout->addSpacing(0);

    // 初始化动画定时器
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(80); // 动画更新间隔
    connect(m_animationTimer, &QTimer::timeout, this, &ListenContent::updateWaveform);
}

void ListenContent::initWaveform()
{
    // 创建波形条
    for (int i = 0; i < m_waveCount; ++i)
    {
        QFrame *bar = new QFrame(this);
        // bar->setMinimumWidth(6);
        // bar->setMaximumWidth(6);
        bar->setStyleSheet("background-color: #3973b6;");
        bar->setFixedWidth(6);
        m_waveBars.append(bar);
        m_waveLayout->addWidget(bar);
    }
}

void ListenContent::updateWaveform()
{
    // 随机更新波形条高度，模拟音频波动
    for (QFrame *bar : m_waveBars)
    {
        // 生成10-80之间的随机高度，中间的条波动更大
        int index = m_waveBars.indexOf(bar);
        int maxHeight = (index == m_waveCount / 2) ? 80 : 50;
        int minHeight = (index == m_waveCount / 2) ? 30 : 5;

        int height = QRandomGenerator::global()->bounded(minHeight, maxHeight);
        bar->setFixedHeight(height);
    }
}

void ListenContent::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 显示时启动动画
    m_animationTimer->start();
}

void ListenContent::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    // 隐藏时停止动画
    m_animationTimer->stop();
}

void ListenContent::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter.drawRect(rect());
}