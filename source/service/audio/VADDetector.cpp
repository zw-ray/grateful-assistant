#include "VADDetector.h"

#include "kernel/Configuration.h"
#include "kernel/Logger.h"
#include "webrtc_vad.h"
#include <cstdint>

VADDetector::VADDetector() : m_inst(nullptr) {}

VADDetector::~VADDetector() {}

bool VADDetector::initialize()
{
    m_inst = WebRtcVad_Create();
    int res = WebRtcVad_Init(m_inst);
    if (res != 0)
    {
        Logger::logError("Failed to initialize webrtc_vad handle.");
        WebRtcVad_Free(m_inst);
        m_inst = nullptr;
        return false;
    }

    res = WebRtcVad_set_mode(m_inst, 2); // 中等灵敏度
    if (res != 0)
    {
        Logger::logError("Failed to set mode for webrtc_vad handle.");
        WebRtcVad_Free(m_inst);
        m_inst = nullptr;
        return false;
    }

    return true;
}

bool VADDetector::isActiveAudio(const std::vector<int16_t> &audioBuffer) const
{
    if (m_inst == nullptr)
    {
        Logger::logError("VADDetector is not initialized.");
        return false;
    }
    if (audioBuffer.empty())
    {
        Logger::logError("Input detected data is empty.");
        return false;
    }
    auto &config = Configuration::getInstance();
    const int32_t sampleRate =
        std::get<int32_t>(config.get("/audio/sample_rate", (int32_t)16000));
    const int32_t sampleWidth =
        std::get<int32_t>(config.get("/audio/sample_width", (int32_t)2));
    const int32_t channels =
        std::get<int32_t>(config.get("/audio/channels", (int32_t)1));
    const int32_t framesPerBuffer = std::get<int32_t>(
        config.get("/audio/frames_per_buffer", (int32_t)3200));
    const int32_t vadFrameDurationMS = std::get<int32_t>(
        config.get("/audio/vad_frame_duration_ms", (int32_t)30));
    // 以下为计算值
    const int32_t frameLength = sampleRate * vadFrameDurationMS / 1000;
    const int32_t times = framesPerBuffer / frameLength;
    int32_t validSlices = 0;
    for (int i = 0; i < times; i++)
    {
        const int16_t *pos = audioBuffer.data() + i * frameLength;

        int res = WebRtcVad_Process(m_inst, sampleRate, audioBuffer.data(),
                                    frameLength);
        if (res == 1)
        {
            validSlices++;
        }
    }
    double activeRatio = validSlices / (double)times;
    return activeRatio > 0.01;
}
