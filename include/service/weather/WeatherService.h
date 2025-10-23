/*******************************************************************************
**     FileName: WeatherService.h
**    ClassName: WeatherService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:41
**  Description:
*******************************************************************************/

#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H

#include <kernel/IService.h>
#include <memory>

class WeatherService : public IService
{
public:
    WeatherService();
    ~WeatherService();

    std::string getName() const override { return "weather"; }

    bool initialize() override;
    bool start() override;
    void shutdown() override;
    bool isInitialized() const override;
    bool isRunning() const override;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class WeatherService

SERVICE_EXPORT_FUNCTION()

#endif // WEATHERSERVICE_H
