#include <kernel/EventBus.h>

// ============ Subscription ==============

Subscription::Subscription(std::function<void()> unsubscribeFunc)
    : m_unsubscribeFunc(std::move(unsubscribeFunc)), m_isSubscribed(true)
{
}
Subscription::~Subscription() { unsubscribe(); }
Subscription::Subscription(Subscription &&other) noexcept
    : m_unsubscribeFunc(std::move(other.m_unsubscribeFunc)),
      m_isSubscribed(other.m_isSubscribed)
{
    other.m_isSubscribed = false;
}

Subscription &Subscription::operator=(Subscription &&other) noexcept
{
    if (this != &other)
    {
        unsubscribe();
        m_unsubscribeFunc = std::move(other.m_unsubscribeFunc);
        m_isSubscribed = other.m_isSubscribed;
        other.m_isSubscribed = false;
    }
    return *this;
}
void Subscription::unsubscribe()
{
    if (m_isSubscribed)
    {
        m_unsubscribeFunc();
        m_isSubscribed = false;
    }
}

// ============== EventBus ==============

EventBus::EventBus() {}

EventBus &EventBus::getInstance()
{
    static EventBus instance;
    return instance;
}

void EventBus::unsubscribe(uint64_t id)
{
    std::unique_lock<std::shared_mutex> lock(m_mutexForSubscription);
    for (auto &it : m_subscriptions)
    {
        auto &vec = it.second;
        auto subscriber =
            std::find_if(vec.begin(), vec.end(),
                         [id](const std::shared_ptr<IEventCallback> &callback)
                         { return callback->id() == id; });
        if (subscriber != vec.end())
        {
            vec.erase(subscriber);
        }
        else
        {
            continue;
        }
        if (vec.empty())
        {
            m_subscriptions.erase(it.first);
        }
        break;
    }
}

// ============== workPool ==============

EventBus::WorkPool::WorkPool(uint32_t numWorkers) : m_stop(false)
{
    if (numWorkers == 0)
    {
        numWorkers = 1;
    }
    for (uint32_t i = 0; i < numWorkers; ++i)
    {
        m_workers.emplace_back(std::thread(&WorkPool::processTask, this));
    }
}
EventBus::WorkPool::~WorkPool() { stop(); }

void EventBus::WorkPool::enqueue(std::function<void()> task)
{
    std::unique_lock<std::mutex> lock(m_mutexForTaskQueue);
    m_taskQueue.push(std::move(task));
    m_condForTaskQueue.notify_one();
}

void EventBus::WorkPool::stop()
{
    m_stop = true;
    m_condForTaskQueue.notify_all();
    for (auto &worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void EventBus::WorkPool::processTask()
{
    while (!m_stop)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutexForTaskQueue);
            m_condForTaskQueue.wait(lock, [this]
                                    { return m_stop || !m_taskQueue.empty(); });
            if (m_stop)
            {
                return;
            }
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }
        if (task)
        {
            task();
        }
    }
}
