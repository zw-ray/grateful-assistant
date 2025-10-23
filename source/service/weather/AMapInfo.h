/*******************************************************************************
**     FileName: AMapInfo.h
**    ClassName: AMapInfo
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 11:18
**  Description:
*******************************************************************************/

#ifndef AMAPINFO_H
#define AMAPINFO_H

#include <string_view>

static constexpr std::string_view kAampBaseUrl = "https://restapi.amap.com";

static constexpr std::string_view kAmapApiKey = "";
// 高德的天气 API 地址
static constexpr std::string_view kWeatherApiPath = "/v3/weather/weatherInfo";

// 高德的 IP 定位 API 地址
static constexpr std::string_view kIpPositionApiPath = "/v3/ip";

#endif // AMAPINFO_H
