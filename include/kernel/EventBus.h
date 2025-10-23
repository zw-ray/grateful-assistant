/*******************************************************************************
**     FileName: EventBus.h
**    ClassName: EventBus
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 22:16
**  Description:
*******************************************************************************/

#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "kernel/Logger.h"
#include <any>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <kernel/KernelExport.h>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief Represents a subscription to an event.
 *
 * The subscription is automatically canceled when this object is destroyed.
 * You can also manually cancel it by calling unsubscribe().
 */
class KERNEL_API Subscription
{
public:
    /**
     * @brief Construct a new Subscription object.
     * @param bus The EventBus instance.
     * @param unsubscribe_func A function that will be called to cancel the
     * subscription.
     */
    Subscription(std::function<void()> unsubscribeFunc);

    /**
     * @brief Destroy the Subscription object and unsubscribe.
     */
    ~Subscription();

    // Prevent copying
    Subscription(const Subscription &) = delete;
    Subscription &operator=(const Subscription &) = delete;

    // Allow moving
    Subscription(Subscription &&other) noexcept;

    Subscription &operator=(Subscription &&other) noexcept;

    /**
     * @brief Manually unsubscribe from the event.
     */
    void unsubscribe();

private:
    std::function<void()> m_unsubscribeFunc;
    bool m_isSubscribed;
};

class KERNEL_API EventBus
{
    EventBus();
    ~EventBus() = default;

public:
    static EventBus &getInstance();
    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;
    EventBus(EventBus &&) = delete;
    EventBus &operator=(EventBus &&) = delete;

    /**
     * @brief Subscribe to an event type with a callback.
     * @tparam EventType The type of the event to subscribe to.
     * @param callback The function to call when the event is published.
     * @return A Subscription object. Keep it alive to maintain the
     * subscription.
     */
    template <typename EventType>
    Subscription on(std::function<void(const EventType &)> callback)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutexForSubscription);
        auto type_index = std::type_index(typeid(EventType));
        auto subIter = m_subscriptions.find(type_index);
        if (subIter == m_subscriptions.end())
        {
            m_subscriptions[type_index] =
                std::vector<std::shared_ptr<IEventCallback>>();
        }
        auto subscriber = std::make_shared<Subscriber<EventType>>(
            false, m_nextId++, std::move(callback));
        m_subscriptions[type_index].push_back(subscriber);
        return Subscription(
            std::bind(&EventBus::unsubscribe, this, subscriber->id()));
    }

    /**
     * @brief Subscribe to an event type with a callback that is called only
     * once.
     * @tparam EventType The type of the event to subscribe to.
     * @param callback The function to call once when the event is published.
     * @return A Subscription object. It can be ignored if you don't need to
     * unsubscribe manually.
     */
    template <typename EventType>
    Subscription once(std::function<void(const EventType &)> callback)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutexForSubscription);
        auto type_index = std::type_index(typeid(EventType));
        auto subIter = m_subscriptions.find(type_index);
        if (subIter == m_subscriptions.end())
        {
            m_subscriptions[type_index] =
                std::vector<std::shared_ptr<IEventCallback>>();
        }
        auto subscriber = std::make_shared<Subscriber<EventType>>(
            true, m_nextId++, std::move(callback));
        m_subscriptions[type_index].push_back(subscriber);
        return Subscription(
            std::bind(&EventBus::unsubscribe, this, subscriber->id()));
    }

    /**
     * @brief Publish an event synchronously.
     * All registered callbacks for this event type are called immediately in
     * the current thread.
     * @tparam EventType The type of the event to publish.
     * @param event The event object to publish.
     */
    template <typename EventType> void publish(const EventType &event)
    {
        std::vector<std::shared_ptr<IEventCallback>> callbacksCopy;
        {
            std::shared_lock<std::shared_mutex> lock(m_mutexForSubscription);
            auto type_index = std::type_index(typeid(EventType));
            auto iter = m_subscriptions.find(type_index);
            if (iter == m_subscriptions.end())
            {
                return;
            }
            callbacksCopy = iter->second;
        }

        // Call each callback with the event
        for (const auto &iter : callbacksCopy)
        {
            iter->invoke(*this, event);
        }
    }

    /**
     * @brief Publish an event asynchronously.
     * The event is queued and processed by a background worker thread.
     * @tparam EventType The type of the event to publish.
     * @param event The event object to publish. It will be copied into the
     * queue.
     */
    template <typename EventType> void publish_async(EventType event)
    {
        std::vector<std::shared_ptr<IEventCallback>> callbacksCopy;
        {
            std::shared_lock<std::shared_mutex> lock(m_mutexForSubscription);
            auto type_index = std::type_index(typeid(EventType));
            auto iter = m_subscriptions.find(type_index);
            if (iter == m_subscriptions.end())
            {
                return;
            }
            callbacksCopy = iter->second;
        }
        for (const auto &iter : callbacksCopy)
        {
            m_workPool.enqueue(
                [this, iter, event]()
                {
                    try
                    {
                        iter->invoke(*this, event);
                    }
                    catch (const std::exception &e)
                    {
                        Logger::logError(
                            "EventBus: Exception in async callback: %s",
                            e.what());
                    }
                    catch (...)
                    {
                        Logger::logError(
                            "EventBus: Unknown exception in async callback");
                    }
                });
        }
    }

    void unsubscribe(uint64_t id);

private:
    class IEventCallback
    {
    public:
        virtual ~IEventCallback() = default;
        virtual uint64_t id() const = 0;
        virtual void invoke(EventBus &bus, const std::any &event) const = 0;
    };

    template <typename EventType> class Subscriber : public IEventCallback
    {
    public:
        Subscriber(bool once, uint64_t id,
                   std::function<void(const EventType &)> callback)
            : m_once(once), m_id(id), m_callback(std::move(callback))
        {
        }

        uint64_t id() const override { return m_id; }

        void invoke(EventBus &bus, const std::any &event) const override
        {
            try
            {
                const EventType &const_event =
                    std::any_cast<const EventType &>(event);
                m_callback(const_event);
                if (m_once)
                {
                    bus.unsubscribe(id());
                }
            }
            catch (const std::exception &e)
            {
                Logger::logError("EventBus: Exception in async callback: {}",
                                 e.what());
            }
            catch (...)
            {
                Logger::logError(
                    "EventBus: Unknown exception in async callback");
            }
        }

    private:
        bool m_once;
        uint64_t m_id;
        std::function<void(const EventType &)> m_callback;
    };

    struct IEventHolder
    {
        virtual ~IEventHolder() = default;
        virtual void publish(EventBus &bus) const = 0;
    };

    template <typename EventType> struct ConcreteEventHolder : IEventHolder
    {
        explicit ConcreteEventHolder(EventType event, bool async = false)
            : m_event(std::move(event)), m_async(async)
        {
        }

        void publish(EventBus &bus) const override
        {
            if (m_async)
            {
                bus.publish_async(m_event);
            }
            else
            {
                bus.publish(m_event);
            }
        }

    private:
        EventType m_event;
        bool m_async;
    };

    class KERNEL_API WorkPool
    {
    public:
        WorkPool(uint32_t numWorkers = std::thread::hardware_concurrency());
        ~WorkPool();

        void enqueue(std::function<void()> task);
        void stop();

    private:
        void processTask();

    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_taskQueue;
        std::mutex m_mutexForTaskQueue;
        std::condition_variable m_condForTaskQueue;
        std::atomic_bool m_stop;
    };

private:
    std::atomic_uint64_t m_nextId{0};
    std::shared_mutex m_mutexForSubscription;
    std::unordered_map<std::type_index,
                       std::vector<std::shared_ptr<IEventCallback>>>
        m_subscriptions;
    std::thread m_workerThread;
    WorkPool m_workPool;
};

#endif // EVENTBUS_H
