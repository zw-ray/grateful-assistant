/*******************************************************************************
**     FileName: IPLocator.h
**    ClassName: IPLocator
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:45
**  Description:
*******************************************************************************/

#ifndef IPLOCATOR_H
#define IPLOCATOR_H

#include <string>

class IPLocator
{
public:
    IPLocator() {}
    virtual ~IPLocator() {}

    virtual std::string getIPLocation() const = 0;

}; // class IPLocator

#endif // IPLOCATOR_H
