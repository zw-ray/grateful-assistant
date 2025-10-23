#include "AMapIPLocator.h"
#include "IPFetcher.h"

#include "AMapInfo.h"
#include <httplib.h>
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <nlohmann/json.hpp>
#include <kernel/Configuration.h>

AMapIPLocator::AMapIPLocator() {}

AMapIPLocator::~AMapIPLocator() {}

std::string AMapIPLocator::getIPLocation() const
{
    using json = nlohmann::json;
    // 1. 获取IP地址
    std::string ip = IPFetcher::getIPAddress();

    httplib::Client cli(kAampBaseUrl.data());
    httplib::Params params;
    std::string apiKey = std::get<std::string>(Configuration::getInstance().get("/weather/amap/api_key", ""));
    params.emplace("key", apiKey);
    params.emplace("ip", ip);

    // 2. 发送请求获取定位信息
    auto res = cli.Get(kIpPositionApiPath.data(), params, httplib::Headers());
    if (res && res->status == 200)
    {
        auto j = json::parse(res->body);
        if (j["status"] == "1")
        {
            return j["adcode"];
        }
    }
    return "";
}
