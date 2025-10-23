/*******************************************************************************
**     FileName: SystemService.h
**    ClassName: SystemService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/06 09:17
**  Description:
*******************************************************************************/

#ifndef SYSTEMSERVICE_H
#define SYSTEMSERVICE_H

#include <kernel/IService.h>

/**
 * @brief 系统服务，用来进行一些系统级别的操作
 * 支持命令系统。
 */
class SystemService : public IService
{
public:
    SystemService();
    ~SystemService();

protected:
}; // class SystemService

SERVICE_EXPORT_FUNCTION()

#endif // SYSTEMSERVICE_H
