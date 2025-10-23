/*******************************************************************************
**     FileName: WeatherCard.h
**    ClassName: WeatherCard
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:37
**  Description: 显示天气的组件
*******************************************************************************/

#ifndef WEATHER_CARD_H
#define WEATHER_CARD_H

#include "Card.h"
#include <QWidget>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/WeatherInfo.h>
#include <memory>

class WeatherCard : public Card
{
    Q_OBJECT
public:
    WeatherCard(QWidget *parent = nullptr);
    ~WeatherCard();

protected:
    void initUI();
    void paintEvent(QPaintEvent *event) override;
    void updateBackgroundByWeather(int tempValue);

signals:
    void
        weatherRequested(const WeatherEvents::WeatherUpdatedEvent &weatherInfo);

protected:
    void updateWeather(const WeatherEvents::WeatherUpdatedEvent &weatherInfo);
protected slots:
    void updateUI();

private:
    struct Data;
    std::unique_ptr<Data> m_data;
};

#endif // WEATHER_CARD_H
