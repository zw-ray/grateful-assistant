/*******************************************************************************
**     FileName: DDNSOrayFetcher.hpp
**    ClassName: DDNSOrayFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 11:04
**  Description:
*******************************************************************************/

#ifndef DDNSORAYFETCHER_H
#define DDNSORAYFETCHER_H

#include "IPFetcher.h"
#include <string>

class DDNSOrayFetcher : public IPFetcher
{
public:
    DDNSOrayFetcher() : IPFetcher("https://ddns.oray.com/checkip") {}
    ~DDNSOrayFetcher() {}

    std::string parseIP(const std::string &responseStr) const override
    {
        // 解析 ddns.oray.com 返回的字符串
        // 格式为：Current IP Address: 119.98.220.41
        // 返回：119.98.220.41
        std::string ip;
        std::regex re("Current IP Address: (\\d+\\.\\d+\\.\\d+\\.\\d+)");
        std::smatch match;
        if (std::regex_search(responseStr, match, re))
        {
            ip = match[1].str();
        }
        return ip;
    }

protected:
}; // class DDNSOrayFetcher

#endif // DDNSORAYFETCHER_H
