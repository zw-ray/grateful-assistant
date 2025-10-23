/*******************************************************************************
**     FileName: MyIpFetcher.hpp
**    ClassName: MyIpFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:59
**  Description:
*******************************************************************************/

#ifndef MYIPFETCHER_H
#define MYIPFETCHER_H

#include "IPFetcher.h"
#include <httplib.h>
#include <string>

class MyIpFetcher : public IPFetcher
{
public:
    MyIpFetcher() : IPFetcher("https://myip.ipip.net") {}
    ~MyIpFetcher() {}

    std::string parseIP(const std::string &responseStr) const
    {
        // 解析 myip.ipip.net 返回的字符串
        // 格式为：当前 IP：119.98.220.41  来自于：中国 湖北 武汉  电信
        // 返回：119.98.220.41
        std::string ip;
        std::regex re("当前 IP：(\\d+\\.\\d+\\.\\d+\\.\\d+)");
        std::smatch match;
        if (std::regex_search(responseStr, match, re))
        {
            ip = match[1].str();
        }
        return ip;
    }
};

#endif // MYIPFETCHER_H
