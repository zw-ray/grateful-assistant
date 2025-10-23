/*******************************************************************************
**     FileName: WeatherFetcher.h
**    ClassName: WeatherFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:46
**  Description:
*******************************************************************************/

#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <kernel/WeatherInfo.h>

class WeatherFetcher
{
public:
    WeatherFetcher() {}
    virtual ~WeatherFetcher() {}

    virtual void fetchWeather(const std::string &cityCode,
                              WeatherInfo &weatherInfo) = 0;

protected:
}; // class WeatherFetcher

#endif // WEATHERFETCHER_H
