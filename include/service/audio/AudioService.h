/*******************************************************************************
**     FileName: AudioService.h
**    ClassName: AudioService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:40
**  Description:
*******************************************************************************/

#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <kernel/IService.h>
#include <memory>

class AudioService : public IService
{
public:
    AudioService();
    ~AudioService();

    std::string getName() const override { return "audio"; }

    bool initialize() override;
    bool start() override;
    void shutdown() override;
    bool isInitialized() const override;
    bool isRunning() const override;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class AudioService

SERVICE_EXPORT_FUNCTION()

#endif // AUDIOSERVICE_H
