/*******************************************************************************
**     FileName: YinghualuoFetcher.hpp
**    ClassName: YinghualuoFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 11:06
**  Description:
*******************************************************************************/

#ifndef YINGHUALUOFETCHER_H
#define YINGHUALUOFETCHER_H

#include "IPFetcher.h"
#include <nlohmann/json.hpp>

class YinghualuoFetcher : public IPFetcher
{
public:
    YinghualuoFetcher() : IPFetcher("https://v4.yinghualuo.cn/bejson") {}
    ~YinghualuoFetcher() {}

    std::string parseIP(const std::string &responseStr) const override
    {
        using json = nlohmann::json;
        // 解析 v4.yinghualuo.cn/bejson 返回的字符串
        // 格式为：{"is_ipv6":false,"ip":"119.98.220.41","location":"湖北省武汉市
        // 电信","area":"湖北省武汉市","isp":"电信","author":"ritaswc@163.com","copyright":"cz88.net
        // 纯真网络"}
        json j = json::parse(responseStr);
        return j["ip"];
    }

protected:
}; // class YinghualuoFetcher

#endif // YINGHUALUOFETCHER_H
