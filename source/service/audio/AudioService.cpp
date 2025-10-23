#include "AudioWorker.h"
#include "PortaudioWrapper.h"
#include "VADDetector.h"
#include "kernel/IService.h"
#include "kernel/Logger.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <service/audio/AudioService.h>

#include <kernel/Configuration.h>
#include <portaudio.h>

struct AudioService::Data
{
    std::atomic_bool initialized{false};
    std::shared_ptr<PortaudioWrapper> audio{nullptr};
    std::shared_ptr<VADDetector> vadDetector{nullptr};
    std::shared_ptr<AudioWorker> worker{nullptr};
    std::atomic_bool running{false};
};

AudioService::AudioService() : m_data(new Data) {}

AudioService::~AudioService() {}

void setupAudioSettings()
{
    auto &config = Configuration::getInstance();
    // 设置音频采样信息

    // 音频采样率
    const int32_t sampleRate = 16000;
    // 每帧字节数，如 16位PCM为2字节
    const int32_t sampleWidth = 2;
    // 单声道
    const int32_t channels = 1;
    // 每缓冲区帧数，2s的缓冲区，
    // framesPerBuffer=sampleRate * 200 / 1000 = 3200；
    const int32_t framesPerBuffer = 3200;
    // vad检测时帧时长，计算方式为：
    // vad帧时长 = 30ms
    // 则frame_length = sampleRate * 30 / 1000 = 480；
    const int32_t vadFrameDurationMS = 30;

    config.set("/audio/sample_rate", sampleRate);
    config.set("/audio/sample_width", sampleWidth);
    config.set("/audio/channels", channels);
    config.set("/audio/frames_per_buffer", framesPerBuffer);
    config.set("/audio/vad_frame_duration_ms", vadFrameDurationMS);
}

bool AudioService::initialize()
{
    // 初始化音频配置
    setupAudioSettings();
    m_data->audio = std::make_shared<PortaudioWrapper>();
    if (!m_data->audio->initialize())
    {
        m_data->initialized = false;
        return false;
    }

    m_data->vadDetector = std::make_shared<VADDetector>();
    if (!m_data->vadDetector->initialize())
    {
        m_data->initialized = false;
        return false;
    }

    m_data->worker =
        std::make_shared<AudioWorker>(m_data->audio, m_data->vadDetector);

    m_data->initialized = true;
    return true;
}

bool AudioService::start()
{
    if (!m_data->initialized)
    {
        Logger::logError("AudioService start failure for not initialized.");
        return false;
    }
    // TODO: start the audio recording and vad detecting.
    if (m_data->worker)
    {
        m_data->worker->start();
    }
    else
    {
        Logger::logError("AudioService start failure for no worker.");
        return false;
    }
    m_data->running = m_data->worker->isRunning();
    return true;
}

void AudioService::shutdown()
{
    if (m_data->initialized)
    {
        if (isRunning())
        {
            m_data->worker->stop();
        }
    }
    Logger::logInfo("AudioService shutdown.");
}

bool AudioService::isInitialized() const { return m_data->initialized; }

bool AudioService::isRunning() const
{
    return m_data->running && m_data->worker->isRunning();
}

IService *createService() { return new AudioService; }

void destroyService(IService *service) { delete service; }
