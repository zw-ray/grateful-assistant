/*******************************************************************************
**     FileName: AMapIPLocator.h
**    ClassName: AMapIPLocator
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:56
**  Description:
*******************************************************************************/

#ifndef AMAPIPLOCATOR_H
#define AMAPIPLOCATOR_H

#include "IPLocator.h"

class AMapIPLocator : public IPLocator
{
public:
    AMapIPLocator();
    ~AMapIPLocator();

    /**
     * @brief 通过IP获取定位信息
     *
     * @return std::string adcode
     */
    std::string getIPLocation() const override;

protected:
}; // class AMapIPLocator

#endif // AMAPIPLOCATOR_H
