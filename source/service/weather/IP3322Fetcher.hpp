/*******************************************************************************
**     FileName: IP3322Fetcher.hpp
**    ClassName: IP3322Fetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 11:06
**  Description:
*******************************************************************************/

#ifndef IP3322FETCHER_H
#define IP3322FETCHER_H

#include "IPFetcher.h"

class IP3322Fetcher : public IPFetcher
{
public:
    IP3322Fetcher() : IPFetcher("https://ip.3322.net") {}
    ~IP3322Fetcher() {}

    std::string parseIP(const std::string &responseStr) const override
    {
        // 解析 ip.3322.net 返回的字符串
        // 格式为：119.98.220.41
        // 返回：119.98.220.41
        return responseStr;
    }
};

#endif // IP3322FETCHER_H
