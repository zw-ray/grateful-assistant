/*******************************************************************************
**     FileName: SyncService.h
**    ClassName: SyncService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 19:33
**  Description:
*******************************************************************************/

#ifndef SYNCSERVICE_H
#define SYNCSERVICE_H

#include <kernel/IService.h>

class SyncService : public IService
{
public:
    SyncService();
    ~SyncService();

    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override;

protected:
}; // class SyncService

#endif // SYNCSERVICE_H
