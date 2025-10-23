/*******************************************************************************
**     FileName: AMapWeatherFetcher.h
**    ClassName: AMapWeatherFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:47
**  Description:
*******************************************************************************/

#ifndef AMAPWEATHERFETCHER_H
#define AMAPWEATHERFETCHER_H

#include "IPLocator.h"
#include "WeatherFetcher.h"
#include <memory>

class AMapWeatherFetcher : public WeatherFetcher
{
public:
    AMapWeatherFetcher();
    ~AMapWeatherFetcher();

    void fetchWeather(const std::string &cityCode,
                      WeatherInfo &weatherInfo) override;

    void setIPLocator(std::shared_ptr<IPLocator> ipLocator);

protected:
    std::shared_ptr<IPLocator> m_ipLocator;
}; // class AMapWeatherFetcher

#endif // AMAPWEATHERFETCHER_H
