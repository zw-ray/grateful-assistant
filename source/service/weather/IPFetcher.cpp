#include "IPFetcher.h"

#include "DDNSOrayFetcher.hpp"
#include "IP3322Fetcher.hpp"
#include "MyIpFetcher.hpp"
#include "YinghualuoFetcher.hpp"
#include <httplib.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

IPFetcher::IPFetcher(const std::string &url) : m_url(url) {}
IPFetcher::~IPFetcher() {}

std::string IPFetcher::getIP() const
{
    if (m_url.empty())
    {
        return "";
    }
    httplib::Client cli(m_url);
    auto res = cli.Get("/");
    if (res && res->status == 200)
    {
        return parseIP(res->body);
    }
    else
    {
        return "";
    }
}

std::string IPFetcher::getIPAddress()
{
    MyIpFetcher myIpFetcher;
    std::string ip = myIpFetcher.getIP();
    if (!ip.empty())
    {
        return ip;
    }
    IP3322Fetcher ip3322Fetcher;
    ip = ip3322Fetcher.getIP();
    if (!ip.empty())
    {
        return ip;
    }
    DDNSOrayFetcher ddnsOrayFetcher;
    ip = ddnsOrayFetcher.getIP();
    if (!ip.empty())
    {
        return ip;
    }
    YinghualuoFetcher yinghualuoFetcher;
    ip = yinghualuoFetcher.getIP();
    if (!ip.empty())
    {
        return ip;
    }
    return "";
}
