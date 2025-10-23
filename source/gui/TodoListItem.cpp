#include "TodoListItem.h"
#include "kernel/entities/Todo.h"
#include <QBrush>
#include <QDateTime>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <chrono>
#include <qfontmetrics.h>

namespace {

QString getStatusText(Todo::Status status)
{
    switch (status)
    {
    case Todo::Status::kNotStarted:
        return "未开始";
    case Todo::Status::kInProgress:
        return "进行中";
    case Todo::Status::kCompleted:
        return "已完成";
    case Todo::Status::kOverdue:
        return "已逾期";
    default:
        return "未知状态";
    }
}

QString getStatusStyle(Todo::Status status)
{
    QString baseStyle = "font-size: 12px; padding: 2px 8px; border-radius: 4px;";

    switch (status)
    {
    case Todo::Status::kNotStarted:
        return baseStyle + " background-color: #c5c5c5; color: #757575;";
    case Todo::Status::kInProgress:
        return baseStyle + " background-color: #eee343; color: #1976d2;";
    case Todo::Status::kCompleted:
        return baseStyle + " background-color: #6ca371; color: #388e3c;";
    case Todo::Status::kOverdue:
        return baseStyle + " background-color: #f59709; color: #d32f2f;";
    default:
        return baseStyle;
    }
}

static QString getPriorityText(Todo::Priority priority)
{
    switch (priority)
    {
    case Todo::Priority::kImportantAndUrgent:
        return u8"高优先级";
    case Todo::Priority::kImportantButNotUrgent:
        return u8"中优先级";
    case Todo::Priority::kNotImportantButUrgent:
        return u8"低优先级";
    case Todo::Priority::kNotImportantAndNotUrgent:
        return u8"无优先级";
    default:
        return u8"未知优先级";
    }
}
QString getPriorityStyle(Todo::Priority priority)
{
    switch (priority)
    {
    case Todo::Priority::kImportantAndUrgent:
        return "font-size: 12px; background-color: #8b2720; color: #fff; padding: 2px 8px; "
               "border-radius: 4px;";
    case Todo::Priority::kImportantButNotUrgent:
        return "font-size: 12px; background-color: #b67109; color: #fff; padding: 2px 8px; "
               "border-radius: 4px;";
    case Todo::Priority::kNotImportantButUrgent:
        return "font-size: 12px; background-color: #b6b079; color: #f57c00; padding: 2px 8px; "
               "border-radius: 4px;";
    case Todo::Priority::kNotImportantAndNotUrgent:
        return "font-size: 12px; background-color: #4caf50; color: #fff; padding: 2px 8px; "
               "border-radius: 4px;";
    default:
        return "font-size: 12px; background-color: #4caf50; color: #fff; padding: 2px 8px; "
               "border-radius: 4px;";
    }
}

} // namespace

TodoListItem::TodoListItem(const Todo &todo, QWidget *parent)
    : QWidget(parent), m_todo(todo), m_leftTextArea(nullptr), m_rightLeftArea(nullptr),
      m_rightRightArea(nullptr)
{
    initUI();
    updateItem(todo); // 初始化显示内容
}

TodoListItem::~TodoListItem() = default;

void TodoListItem::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    // this->setFixedHeight(40);
    // 初始化渐变
    m_gradient = QLinearGradient(0, 0, width(), height());
    updateGradient();

    if (parent())
    {
        QWidget *parentWidget = qobject_cast<QWidget *>(parent());
        this->setMaximumWidth(parentWidget->width());
    }

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(20);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // 左侧文本区域
    m_leftTextArea = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftTextArea);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(8);
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText(QString::fromUtf8(m_todo.title.c_str()));
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin: 4pt; padding: 0;");
    leftLayout->addWidget(titleLabel);
    QLabel *contentLabel = new QLabel(this);
    contentLabel->setText(QString::fromUtf8(m_todo.content.c_str()));
    QFontMetrics fontMetrics(contentLabel->font());
    QString elidedText =
        fontMetrics.elidedText(contentLabel->text(), Qt::ElideMiddle, this->width() / 2);
    contentLabel->setText(elidedText);
    contentLabel->setStyleSheet("font-size: 12px; color: #757575; margin: 4pt; padding: 0;");
    leftLayout->addWidget(contentLabel);

    // 右侧的左边区域
    m_rightLeftArea = new QWidget(this);
    QVBoxLayout *rightLeftLayout = new QVBoxLayout(m_rightLeftArea);
    rightLeftLayout->setContentsMargins(8, 8, 8, 8);
    rightLeftLayout->setSpacing(8);
    QLabel *priorityLabel = new QLabel(this);
    priorityLabel->setText(getPriorityText(m_todo.priority));
    priorityLabel->setStyleSheet(getPriorityStyle(m_todo.priority));
    rightLeftLayout->addWidget(priorityLabel);
    QLabel *statusLabel = new QLabel(this);
    statusLabel->setText(getStatusText(m_todo.status));
    statusLabel->setStyleSheet(getStatusStyle(m_todo.status));
    rightLeftLayout->addWidget(statusLabel);

    // 右侧的右边区域
    m_rightRightArea = new QWidget(this);
    QVBoxLayout *rightRightLayout = new QVBoxLayout(m_rightRightArea);
    rightRightLayout->setContentsMargins(8, 8, 8, 8);
    rightRightLayout->setSpacing(8);
    QPushButton *completeButton = new QPushButton(this);
    completeButton->setStyleSheet(
        "font-size: 12px; padding: 2px 8px; border-radius: 4px; "
        "background-color: #4caf50; color: #fff; hover: #36b43dff; disabled: #9e9e9e; "
        "click: #2e7d32ff;");
    switch (m_todo.status)
    {
    case Todo::Status::kNotStarted:
    {
        completeButton->setText("开始");
        completeButton->setEnabled(true);
    }
    break;
    case Todo::Status::kInProgress:
    {
        completeButton->setText("完成");
        completeButton->setEnabled(true);
    }
    break;
    case Todo::Status::kCompleted:
    {
        completeButton->setText("已完成");
        completeButton->setEnabled(false);
    }
    break;
    case Todo::Status::kOverdue:
    {
        completeButton->setText("重新开始");
        completeButton->setEnabled(true);
    }
    break;
    default:
    {
        completeButton->setText("完成");
        completeButton->setEnabled(true);
    }
    break;
    }
    rightRightLayout->addWidget(completeButton);
    // 连接完成按钮点击信号到槽函数
    connect(completeButton, &QPushButton::clicked, this, &TodoListItem::onCompleteButtonClicked);

    mainLayout->addWidget(m_leftTextArea);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_rightLeftArea);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_rightRightArea);
}

float TodoListItem::calculateProgress() const
{
    if (m_todo.status == Todo::Status::kCompleted)
    {
        return 100;
    }

    // 假设Todo有deadline字段表示截止时间（时间戳）
    auto now = std::chrono::system_clock::now();
    auto deadline = m_todo.dueTime;

    // 处理未设置截止时间的情况
    if (deadline.time_since_epoch().count() <= 0)
    {
        return m_todo.status == Todo::Status::kInProgress ? 30 : 0;
    }

    // 计算总时间跨度（秒）
    auto totalTime =
        std::chrono::duration_cast<std::chrono::seconds>(deadline - m_todo.createdAt).count();

    // 计算已用时间（秒）
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                           now + std::chrono::hours(4) - m_todo.createdAt)
                           .count();

    // 计算进度（0-100%）
    float progress = (float)elapsedTime / (float)totalTime;

    // 限制在0-1之间（已完成状态在上面单独处理）
    return qBound(0.0f, progress, 1.0f);
}

void TodoListItem::updateItem(const Todo &todo)
{
    m_todo = todo;
    update();
}

void TodoListItem::updateGradient()
{
    // 计算当前进度
    float progress = calculateProgress();

    // 更新渐变范围（随控件尺寸变化）
    m_gradient.setStart(0, height() / 2);
    m_gradient.setFinalStop(width(), height() / 2);

    // 清除之前的颜色停止点
    m_gradient.setColorAt(0.0, QColor());
    m_gradient.setColorAt(1.0, QColor());

    // 根据进度计算颜色过渡
    // 进度低（0.0）时偏绿色，进度高（1.0）时偏红色
    int hue1, hue2;
    int saturation = 180; // 适中的饱和度
    int lightness = 200;  // 较高的明度，使颜色更柔和

    // 计算主色调变化：绿色(120) -> 黄色(60) -> 红色(0)
    int mainHue = 120 - static_cast<int>(progress * 120);

    // 设置渐变的两个端点颜色，随进度变化
    if (progress < 0.5)
    {
        // 前半段：绿色到黄色渐变
        hue1 = 120;
        hue2 = 60;
    }
    else
    {
        // 后半段：黄色到红色渐变
        hue1 = 60;
        hue2 = 0;
    }

    // 添加渐变颜色点，创建更丰富的过渡效果
    m_gradient.setColorAt(0.0, QColor::fromHsl(hue1, saturation, lightness));
    m_gradient.setColorAt(0.5, QColor::fromHsl(mainHue, saturation, lightness));
    m_gradient.setColorAt(1.0, QColor::fromHsl(hue2, saturation, lightness));
}

void TodoListItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    // 绘制渐变背景
    painter.setBrush(m_gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 8, 8); // 带圆角的矩形
}

void TodoListItem::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // 当控件尺寸变化时，更新渐变范围
    updateGradient();
}

void TodoListItem::onCompleteButtonClicked()
{
    switch (m_todo.status)
    {
    case Todo::Status::kNotStarted:
    {
        m_todo.status = Todo::Status::kCompleted;
    }
    break;
    case Todo::Status::kInProgress:
    {
        m_todo.status = Todo::Status::kCompleted;
    }
    break;
    case Todo::Status::kCompleted:
    {
    }
    break;
    case Todo::Status::kOverdue:
    {
        m_todo.status = Todo::Status::kInProgress;
    }
    break;
    default:
    {
        m_todo.status = Todo::Status::kCompleted;
    }
    break;
    }
}
