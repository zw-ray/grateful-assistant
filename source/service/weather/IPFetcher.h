/*******************************************************************************
**     FileName: IPFetcher.h
**    ClassName: IPFetcher
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:57
**  Description:
*******************************************************************************/

#ifndef IPFETCHER_H
#define IPFETCHER_H

#include <httplib.h>
#include <string>

class IPFetcher
{
public:
    IPFetcher(const std::string &url);
    virtual ~IPFetcher();

    virtual std::string getIP() const;
    virtual std::string parseIP(const std::string &responseStr) const = 0;

    static std::string getIPAddress();

protected:
    std::string m_url;
}; // class IPFetcher

#endif // IPFETCHER_H
