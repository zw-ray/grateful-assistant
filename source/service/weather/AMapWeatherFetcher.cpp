#include "AMapWeatherFetcher.h"
#include "AMapIPLocator.h"
#include "AMapInfo.h"
#include "kernel/Logger.h"

#include <httplib.h>
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <nlohmann/json.hpp>

#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <string>
#include <kernel/Configuration.h>

AMapWeatherFetcher::AMapWeatherFetcher() : m_ipLocator(nullptr)
{
    m_ipLocator = std::make_shared<AMapIPLocator>();
}

AMapWeatherFetcher::~AMapWeatherFetcher() {}

void AMapWeatherFetcher::fetchWeather(const std::string &cityCode,
                                      WeatherInfo &weatherInfo)
{
    if (m_ipLocator == nullptr)
    {
        return;
    }

    std::string adcode = m_ipLocator->getIPLocation();
    if (adcode.empty())
    {
        return;
    }

    httplib::Client cli(kAampBaseUrl.data());
    httplib::Params params;
    std::string apiKey = std::get<std::string>(Configuration::getInstance().get("/weather/amap/api_key", ""));
    params.emplace("key", apiKey);
    params.emplace("city", adcode);
    params.emplace("extensions", "base");
    params.emplace("output", "JSON");
    auto res = cli.Get(kWeatherApiPath.data(), params, httplib::Headers());
    if (res && res->status == 200)
    {
        using json = nlohmann::json;
        auto j = json::parse(res->body);
        if (j["status"] == "1")
        {
            auto lives = j["lives"];
            for (auto &live : lives)
            {
                weatherInfo.province = live["province"];
                weatherInfo.city = live["city"];
                weatherInfo.weather = live["weather"];
                weatherInfo.temp = live["temperature"];
                weatherInfo.windDirection = live["winddirection"];
                weatherInfo.windPower = live["windpower"];
                weatherInfo.humidity = live["humidity"];
                weatherInfo.reportTime = live["reporttime"];

                Logger::logInfo(
                    "Weather info: province:{}, city:{}, weather:{}, temp:{}, "
                    "windDirection:{}, windPower:{}, humidity:{}, "
                    "reportTime:{}",
                    weatherInfo.province, weatherInfo.city, weatherInfo.weather,
                    weatherInfo.temp, weatherInfo.windDirection,
                    weatherInfo.windPower, weatherInfo.humidity,
                    weatherInfo.reportTime);
            }
        }
    }
}

void AMapWeatherFetcher::setIPLocator(std::shared_ptr<IPLocator> ipLocator)
{
    m_ipLocator = ipLocator;
}
