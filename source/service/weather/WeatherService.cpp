#include "AMapIPLocator.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <kernel/IService.h>
#include <memory>
#include <service/weather/WeatherService.h>
#include <thread>

#include "AMapWeatherFetcher.h"
#include "IPLocator.h"
#include "WeatherFetcher.h"
#include "kernel/Logger.h"
#include "kernel/WeatherInfo.h"
#include <kernel/EventBus.h>
#include <kernel/Events.h>

struct WeatherService::Data
{
    std::shared_ptr<WeatherFetcher> fetcher;
    std::shared_ptr<IPLocator> locator;
    std::atomic_bool running;
    std::thread workerThread;
    Subscription requestSubscription;
    bool initialized;

    Data() : running(false), initialized(false), requestSubscription([]() {}) {}
};

WeatherService::WeatherService() : m_data(nullptr) {}

WeatherService::~WeatherService() { shutdown(); }

void updateWeather(const WeatherEvents::WeatherRequestEvent &event)
{
    AMapWeatherFetcher fetcher;
    AMapIPLocator locator;
    auto location = locator.getIPLocation();
    WeatherInfo weatherInfo;
    fetcher.fetchWeather(location, weatherInfo);

    WeatherEvents::WeatherUpdatedEvent updateEvent;
    updateEvent.time = std::time(nullptr);
    updateEvent.city = weatherInfo.city;
    updateEvent.weather_info = weatherInfo;
    EventBus::getInstance().publish_async<WeatherEvents::WeatherUpdatedEvent>(
        updateEvent);
}

bool WeatherService::initialize()
{
    m_data = std::make_unique<Data>();
    m_data->running = false;
    m_data->fetcher = std::make_shared<AMapWeatherFetcher>();
    m_data->locator = std::make_shared<AMapIPLocator>();
    m_data->initialized = true;

    // 订阅天气请求事件
    m_data->requestSubscription =
        EventBus::getInstance().on<WeatherEvents::WeatherRequestEvent>(
            updateWeather);
    return true;
}

bool WeatherService::start()
{
    if (!m_data->running)
    {
        m_data->running = true;
        auto func = [&]()
        {
            auto startTime = std::chrono::system_clock::now();
            constexpr uint64_t duration = 60 * 60 * 3; // 3小时，s
            while (m_data->running)
            {
                auto now = std::chrono::system_clock::now();
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(
                                now - startTime)
                                .count();
                if (diff >= duration)
                {
                    updateWeather({});
                    startTime = now;
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        };
        m_data->workerThread = std::thread(func);
    }
    return true;
}

void WeatherService::shutdown()
{
    if (m_data)
    {
        m_data->requestSubscription.unsubscribe();
        m_data->running = false;
        if (m_data->workerThread.joinable())
        {
            m_data->workerThread.join();
        }
    }
    Logger::logInfo("WeatherService shutdown.");
}

bool WeatherService::isInitialized() const
{
    return m_data && m_data->initialized;
}

bool WeatherService::isRunning() const { return m_data && m_data->running; }

IService *createService() { return new WeatherService(); }

void destroyService(IService *service) { delete service; }
