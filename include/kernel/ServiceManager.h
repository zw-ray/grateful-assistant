/*******************************************************************************
**     FileName: ServiceManager.h
**    ClassName: ServiceManager
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 15:01
**  Description:
*******************************************************************************/

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <kernel/IService.h>
#include <kernel/KernelExport.h>
#include <memory>
#include <vector>


class KERNEL_API ServiceManager
{
    ServiceManager();
    ~ServiceManager();

public:
    static ServiceManager &getInstance();

    std::vector<std::shared_ptr<IService>> loadSystemServices();
    void unloadSystemServices();

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class ServiceManager

#endif // SERVICEMANAGER_H
