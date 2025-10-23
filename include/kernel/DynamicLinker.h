/*******************************************************************************
**     FileName: DynamicLinker.h
**    ClassName: DynamicLinker
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 13:09
**  Description:
*******************************************************************************/

#ifndef DYNAMICLINKER_H
#define DYNAMICLINKER_H

#include <kernel/Extension.h>
#include <kernel/IService.h>
#include <kernel/KernelExport.h>
#include <memory>
#include <string>

class KERNEL_API DynamicLinker
{
    DynamicLinker();
    ~DynamicLinker();

public:
    static DynamicLinker &getInstance();

    std::shared_ptr<IService> loadService(const std::string &serviceName);
    std::shared_ptr<Extension> loadExtension(const std::string &extensionName);

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class DynamicLinker

#endif // DYNAMICLINKER_H
