#include <QFile>
#include <QGridLayout>
#include <QList>
#include <QPoint>
#include <QQueue>
#include <QTimer>
#include <QVBoxLayout>
#include <functional>
#include <gui/MainWindow.h>
#include <kernel/ServiceManager.h>


#include "DateTimeCard.h"
#include "FloatMessage.h"
#include "ListenContent.h"
#include "TodoInput.h"
#include "TodoList.h"
#include "WeatherCard.h"
#include "kernel/Configuration.h"

#include <kernel/EventBus.h>
#include <kernel/Events.h>

struct MainWindow::Data
{
    DateTimeCard *time;
    WeatherCard *weather;
    TodoList *todoList;
    TodoInput *todoInput;
    ListenContent *listenContent;

    Subscription messageSubscription;
    Subscription validWakeWordSubscription;        // 唤醒词校验成功订阅
    Subscription contentRecordingDoneSubscription; // 内容录音完成订阅

    QQueue<QString> messageQueue;
    QList<FloatMessage *> activeMessages;
    const int MAX_MESSAGES = 3;     // 最大同时显示的消息数
    const int MESSAGE_SPACING = 10; // 消息之间的间距
    QTimer *msgTimer;

    Data()
        : time(nullptr), weather(nullptr), todoList(nullptr),
          todoInput(nullptr), messageSubscription([]() {}),
          validWakeWordSubscription([]() {}),
          contentRecordingDoneSubscription([]() {}), msgTimer(nullptr)
    {
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_data(std::make_unique<Data>())
{
    initUI();
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_data->messageSubscription.unsubscribe();
    m_data->validWakeWordSubscription.unsubscribe();
    m_data->msgTimer->stop();
    ServiceManager::getInstance().unloadSystemServices();
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);
    const qreal pixelRatio = devicePixelRatioF();
    for (auto msg : m_data->activeMessages)
    {
        const QRect mainWindowGeometry = this->geometry();
        QPoint topRight = mainWindowGeometry.topRight();
        for (int i = 0; i < m_data->activeMessages.size(); ++i)
        {
            auto msg = m_data->activeMessages[i];
            msg->setPosition(topRight.x(), topRight.y());
            topRight.setY(topRight.y() + msg->height() +
                          m_data->MESSAGE_SPACING * pixelRatio);
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_data->listenContent)
    {
        m_data->listenContent->resize(this->size());
    }
}
void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}



void MainWindow::initUI()
{
    // 设置窗口标题
    setWindowTitle(tr("Grate Assistant"));

    // 设置窗口大小和样式
    resize(1024, 600);

    // 设置左侧组件
    QVBoxLayout *leftLayout = initLeftComponent();

    // 设置右侧组件
    QVBoxLayout *rightLayout = initRightComponent();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addLayout(leftLayout, 0, 0);
    mainLayout->addLayout(rightLayout, 0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 2);

    // 设置主窗口布局
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    m_data->messageSubscription =
        EventBus::getInstance().on<SystemEvents::SystemMessageEvent>(std::bind(
            &MainWindow::onSystemMessage, this, std::placeholders::_1));
    m_data->validWakeWordSubscription =
        EventBus::getInstance().on<AIEvents::ValidWakeWordEvent>(std::bind(
            &MainWindow::onValidWakeWord, this, std::placeholders::_1));
    m_data->contentRecordingDoneSubscription =
        EventBus::getInstance().on<AudioEvents::AudioContentRecordingDoneEvent>(
            std::bind(&MainWindow::onContentRecordingDone, this,
                      std::placeholders::_1));

    connect(this, &MainWindow::systemMessageIn, this,
            &MainWindow::handleSystemMessageIn);
    connect(this, &MainWindow::triggerShowListenContent, this,
            &MainWindow::onTriggerShowListenContent);
    connect(this, &MainWindow::triggerHideListenContent, this,
            &MainWindow::onTriggerHideListenContent);

    m_data->msgTimer = new QTimer(this);
    m_data->msgTimer->setInterval(200);
    connect(m_data->msgTimer, &QTimer::timeout, this,
            &MainWindow::updateSystemMessage);
    m_data->msgTimer->start();

    m_data->listenContent = new ListenContent(this);
    m_data->listenContent->resize(this->size());
    m_data->listenContent->hide();
}

QVBoxLayout *MainWindow::initLeftComponent()
{
    // 创建时间组件
    m_data->time = new DateTimeCard(this);
    // 创建天气组件
    m_data->weather = new WeatherCard(this);
    // 创建左侧布局
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(m_data->time);
    leftLayout->addWidget(m_data->weather);
    return leftLayout;
}

QVBoxLayout *MainWindow::initRightComponent()
{
    // 创建待办事项组件
    m_data->todoList = new TodoList(this);

    // 创建待办事项输入组件
    m_data->todoInput = new TodoInput(this);

    // 创建右侧布局
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_data->todoList);
    rightLayout->addWidget(m_data->todoInput);
    return rightLayout;
}

void MainWindow::handleSystemMessageIn(const QString &message)
{
    m_data->messageQueue.enqueue(message);
    if (!m_data->msgTimer->isActive())
    {
        m_data->msgTimer->start();
    }
}

void MainWindow::updateSystemMessage()
{
    if (m_data->messageQueue.isEmpty())
    {
        m_data->msgTimer->stop();
        return;
    }

    if (m_data->activeMessages.size() >= m_data->MAX_MESSAGES)
    {
        return;
    }

    QString message = m_data->messageQueue.dequeue();

    FloatMessage *messageWidget = new FloatMessage(message, this);
    connect(messageWidget, &FloatMessage::closed,
            [this, messageWidget]()
            {
                const qreal pixelRatio = devicePixelRatioF();
                m_data->activeMessages.removeOne(messageWidget);
                // 更新其他消息的位置
                const QRect mainWindowGeometry = this->geometry();
                QPoint topRight = mainWindowGeometry.topRight();
                for (int i = 0; i < m_data->activeMessages.size(); ++i)
                {
                    auto msg = m_data->activeMessages[i];
                    msg->setPosition(topRight.x(), topRight.y());
                    topRight.setY(topRight.y() + msg->height() +
                                  m_data->MESSAGE_SPACING * pixelRatio);
                }
            });
    // 计算新消息的位置
    const QRect mainWindowGeometry = this->geometry();
    QPoint topRight = mainWindowGeometry.topRight();
    for (int i = 0; i < m_data->activeMessages.size(); ++i)
    {
        auto prevMsg = m_data->activeMessages[i];
        topRight.setY(topRight.y() + prevMsg->height() +
                      m_data->MESSAGE_SPACING);
    }
    messageWidget->setPosition(topRight.x(), topRight.y());
    m_data->activeMessages.append(messageWidget);
}

void MainWindow::onTriggerShowListenContent()
{
    if (m_data->listenContent)
    {
        m_data->listenContent->resize(this->size());
        m_data->listenContent->show();
    }
}

void MainWindow::onTriggerHideListenContent()
{
    if (m_data->listenContent)
    {
        m_data->listenContent->hide();
    }
}

void MainWindow::onValidWakeWord(const AIEvents::ValidWakeWordEvent &event)
{
    emit triggerShowListenContent();
}

void MainWindow::onContentRecordingDone(
    const AudioEvents::AudioContentRecordingDoneEvent &event)
{
    emit triggerHideListenContent();
}

void MainWindow::onSystemMessage(const SystemEvents::SystemMessageEvent &event)
{
    if (!m_data->msgTimer->isActive())
    {
        m_data->msgTimer->start();
    }
    emit systemMessageIn(QString::fromUtf8(event.message.c_str()));
}
