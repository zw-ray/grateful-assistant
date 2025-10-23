/*******************************************************************************
**     FileName: WeatherInfo.h
**    ClassName: WeatherInfo
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:43
**  Description:
*******************************************************************************/

#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include <kernel/KernelExport.h>
#include <string>

struct WeatherInfo
{
    std::string province;      // 省份
    std::string city;          // 城市
    std::string weather;       // 天气现象（汉字）
    std::string temp;          // 温度
    std::string windDirection; // 风向
    std::string windPower;     // 风力
    std::string humidity;      // 湿度
    std::string reportTime;    // 发布时间

    WeatherInfo() {}
    WeatherInfo(const std::string &province, const std::string &city,
                const std::string &weather, const std::string &temp,
                const std::string &windDirection, const std::string &windPower,
                const std::string &humidity, const std::string &reportTime)
        : province(province), city(city), weather(weather), temp(temp),
          windDirection(windDirection), windPower(windPower),
          humidity(humidity), reportTime(reportTime)
    {
    }

    WeatherInfo(const WeatherInfo &other)
        : province(other.province), city(other.city), weather(other.weather),
          temp(other.temp), windDirection(other.windDirection),
          windPower(other.windPower), humidity(other.humidity),
          reportTime(other.reportTime)
    {
    }

    WeatherInfo &operator=(const WeatherInfo &other)
    {
        if (this != &other)
        {
            province = other.province;
            city = other.city;
            weather = other.weather;
            temp = other.temp;
            windDirection = other.windDirection;
            windPower = other.windPower;
            humidity = other.humidity;
            reportTime = other.reportTime;
        }
        return *this;
    }

    bool operator==(const WeatherInfo &other) const
    {
        return province == other.province && city == other.city &&
               weather == other.weather && temp == other.temp &&
               windDirection == other.windDirection &&
               windPower == other.windPower && humidity == other.humidity &&
               reportTime == other.reportTime;
    }
    bool operator!=(const WeatherInfo &other) const
    {
        return !(*this == other);
    }
};

#endif // WEATHERINFO_H
