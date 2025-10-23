#include "WeatherCard.h"
#include <functional>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/WeatherInfo.h>

#include <QColor>
#include <QEventLoop>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <QPainterPath>
#include <kernel/Logger.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct WeatherCard::Data
{
    WeatherInfo weatherInfo; // 天气信息
    std::string curPosition; // 当前位置信息
    std::string curCityCode;
    Subscription weatherSubscription;

    QFont font;                      // 字体
    QLabel *posLabel = nullptr;      // 显示位置
    QLabel *tempLabel = nullptr;     // 显示温度
    QLabel *weatherLabel = nullptr;  // 显示天气现象
    QLabel *windLabel = nullptr;     // 显示风向风力
    QLabel *humidityLabel = nullptr; // 显示湿度

    // 动态背景相关
    QLinearGradient gradient;                     // 线性渐变背景
    QColor startColor;                            // 渐变起始颜色
    QColor endColor;                              // 渐变结束颜色
    QPropertyAnimation *colorAnimation = nullptr; // 颜色动画

    Data() : weatherSubscription([]() {}) {}

    ~Data() { delete colorAnimation; }
};

WeatherCard::WeatherCard(QWidget *parent)
    : Card(parent), m_data(std::make_unique<Data>())
{
    initUI();
}

WeatherCard::~WeatherCard() { m_data->weatherSubscription.unsubscribe(); }

void WeatherCard::initUI()
{
    // 设置窗口标题
    setWindowTitle(tr("Weather"));
    setObjectName("WeatherCard");

    // 启用背景透明和自动填充
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(true);

    // 设置初始背景颜色（默认温暖天气）
    m_data->startColor = QColor(255, 245, 235, 180);
    m_data->endColor = QColor(255, 228, 196, 180);

    // 初始化渐变和动画
    m_data->gradient.setStart(0, 0);
    m_data->gradient.setFinalStop(0, 1);

    // 创建颜色动画
    m_data->colorAnimation = new QPropertyAnimation(this, "startColor");
    m_data->colorAnimation->setDuration(1500); // 动画持续时间1.5秒

    int fontId = QFontDatabase::addApplicationFont(
        ":/fonts/SauceCodeProNerdFont-Regular.ttf");
    if (fontId == -1)
    {
        Logger::logError(
            "Failed to load font: SauceCodeProNerdFont-Regular.ttf");
        return;
    }
    m_data->font = QFont("SauceCodePro Nerd Font");

    if (m_data->posLabel == nullptr)
    {
        m_data->posLabel = new QLabel(this);
    }
    m_data->posLabel->setObjectName("posLabel");
    m_data->posLabel->setAlignment(Qt::AlignCenter);
    if (m_data->tempLabel == nullptr)
    {
        m_data->tempLabel = new QLabel(this);
    }
    m_data->tempLabel->setObjectName("tempLabel");
    m_data->tempLabel->setFont(m_data->font);
    m_data->tempLabel->setAlignment(Qt::AlignCenter);
    if (m_data->weatherLabel == nullptr)
    {
        m_data->weatherLabel = new QLabel(this);
    }
    m_data->weatherLabel->setObjectName("weatherLabel");
    m_data->weatherLabel->setFont(m_data->font);
    m_data->weatherLabel->setAlignment(Qt::AlignCenter);
    if (m_data->windLabel == nullptr)
    {
        m_data->windLabel = new QLabel(this);
    }
    m_data->windLabel->setObjectName("windLabel");
    m_data->windLabel->setFont(m_data->font);
    m_data->windLabel->setAlignment(Qt::AlignCenter);
    if (m_data->humidityLabel == nullptr)
    {
        m_data->humidityLabel = new QLabel(this);
    }
    m_data->humidityLabel->setObjectName("humidityLabel");
    m_data->humidityLabel->setFont(m_data->font);
    m_data->humidityLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_data->posLabel);
    layout->addWidget(m_data->tempLabel);
    QHBoxLayout *hboxLayout = new QHBoxLayout();
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_data->weatherLabel);
    hboxLayout->addWidget(m_data->windLabel);
    hboxLayout->addWidget(m_data->humidityLabel);
    hboxLayout->addStretch();
    hboxLayout->setSpacing(3);
    layout->addLayout(hboxLayout);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    connect(
        this,
        SIGNAL(weatherRequested(const WeatherEvents::WeatherUpdatedEvent &)),
        this, SLOT(updateUI()));

    auto call =
        std::bind(&WeatherCard::updateWeather, this, std::placeholders::_1);
    m_data->weatherSubscription =
        EventBus::getInstance().on<WeatherEvents::WeatherUpdatedEvent>(call);
    WeatherEvents::WeatherRequestEvent reqEvent;

    EventBus::getInstance().publish_async<WeatherEvents::WeatherRequestEvent>(
        reqEvent);
}

void WeatherCard::updateWeather(
    const WeatherEvents::WeatherUpdatedEvent &weatherInfo)
{
    m_data->weatherInfo = weatherInfo.weather_info;
    Logger::logInfo("Weather info: province:{}, city:{}, weather:{}, temp:{}, "
                    "windDirection:{}, windPower:{}, humidity:{}, "
                    "reportTime:{}",
                    m_data->weatherInfo.province, m_data->weatherInfo.city,
                    m_data->weatherInfo.weather, m_data->weatherInfo.temp,
                    m_data->weatherInfo.windDirection,
                    m_data->weatherInfo.windPower, m_data->weatherInfo.humidity,
                    m_data->weatherInfo.reportTime);
    emit weatherRequested(weatherInfo);
}

// 重写paintEvent绘制渐变背景
void WeatherCard::paintEvent(QPaintEvent *event)
{
    // 先调用父类的paintEvent以保持原有功能
    Card::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置渐变区域为整个窗口
    m_data->gradient.setStart(0, 0);
    m_data->gradient.setFinalStop(0, height());
    m_data->gradient.setColorAt(0, m_data->startColor);
    m_data->gradient.setColorAt(1, m_data->endColor);

    // 创建圆角矩形路径
    QPainterPath path;
    QRect rect = this->rect();
    int radius = 8; // 圆角半径
    path.addRoundedRect(rect, radius, radius);

    // 填充渐变背景
    painter.fillPath(path, m_data->gradient);
}

void WeatherCard::updateUI()
{
    // 设置位置标签
    m_data->posLabel->setText(QString::fromStdString(
        m_data->weatherInfo.province + " - " + m_data->weatherInfo.city));

    // 设置温度标签
    int tempValue = QString::fromStdString(m_data->weatherInfo.temp).toInt();
    QString iconPath;
    if (tempValue >= 30)
    {
        iconPath = ":/icons/weather/high_temperature_hot.svg";
    }
    else if (tempValue <= 0)
    {
        iconPath = ":/icons/weather/low_temperature_cold.svg";
    }
    QString text = QString::fromStdString(m_data->weatherInfo.temp + "°C");
    if (!iconPath.isEmpty())
    {
        m_data->tempLabel->setTextFormat(Qt::RichText);
        QString html = R"(
        <div style="font-size: 16pt;">
            <span style="vertical-align: middle;"><img src='%1' width='48' height='48'>%2</span>
        </div>
        )";
        text = html.arg(iconPath).arg(text);
    }
    m_data->tempLabel->setText(text);

    // 设置天气现象标签
    m_data->weatherLabel->setText(
        QString::fromStdString(m_data->weatherInfo.weather));

    // 设置风向风力标签
    m_data->windLabel->setText(
        QString::fromStdString(m_data->weatherInfo.windDirection + " " +
                               m_data->weatherInfo.windPower));

    // 设置湿度标签
    m_data->humidityLabel->setText(
        QString::fromStdString(m_data->weatherInfo.humidity + "%"));

    // 根据天气和温度设置动态背景
    updateBackgroundByWeather(tempValue);
}

// 根据天气和温度更新背景颜色
void WeatherCard::updateBackgroundByWeather(int tempValue)
{
    QColor newStartColor, newEndColor;
    std::string weather = m_data->weatherInfo.weather;

    // 根据温度和天气类型设置不同的背景颜色
    if (weather.find("雨") != std::string::npos ||
        weather.find("雪") != std::string::npos ||
        weather.find("阴") != std::string::npos)
    {
        // 阴雨天气 - 蓝色系
        newStartColor = QColor(220, 230, 245, 180);
        newEndColor = QColor(180, 200, 230, 180);
    }
    else if (tempValue >= 30)
    {
        // 高温天气 - 橙红色系
        newStartColor = QColor(255, 230, 210, 180);
        newEndColor = QColor(255, 200, 170, 180);
    }
    else if (tempValue <= 5)
    {
        // 低温天气 - 浅蓝色系
        newStartColor = QColor(230, 245, 255, 180);
        newEndColor = QColor(200, 230, 255, 180);
    }
    else
    {
        // 舒适天气 - 浅绿色系
        newStartColor = QColor(220, 245, 220, 180);
        newEndColor = QColor(180, 230, 180, 180);
    }

    // 应用颜色变化动画
    if (m_data->colorAnimation)
    {
        m_data->colorAnimation->stop();
        m_data->colorAnimation->setStartValue(m_data->startColor);
        m_data->colorAnimation->setEndValue(newStartColor);
        m_data->colorAnimation->start();
    }

    // 直接更新结束颜色
    m_data->endColor = newEndColor;

    // 触发重绘
    update();
}
