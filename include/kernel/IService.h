/*******************************************************************************
**     FileName: IService.h
**    ClassName: IService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:09
**  Description:
*******************************************************************************/

#ifndef ISERVICE_H
#define ISERVICE_H

#include <kernel/KernelExport.h>
#include <string>

class KERNEL_API IService
{
public:
    virtual ~IService() = 0;

    virtual std::string getName() const = 0;
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
    virtual bool isRunning() const = 0;

}; // class IService

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#ifndef SERVICE_EXPORT
#define SERVICE_EXPORT __declspec(dllexport)
#endif
#else
#ifndef SERVICE_EXPORT
#define SERVICE_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef SERVICE_EXPORT_FUNCTION
#define SERVICE_EXPORT_FUNCTION()                                              \
    extern "C" SERVICE_EXPORT IService *createService();                       \
    extern "C" SERVICE_EXPORT void destroyService(IService *service);
#endif // SERVICE_EXPORT_FUNCTION

#endif // ISERVICE_H
