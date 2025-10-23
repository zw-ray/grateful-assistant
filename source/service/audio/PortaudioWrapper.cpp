// audio/PortaudioWrapper.cpp
#include "PortaudioWrapper.h"
#include "kernel/Logger.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <kernel/Configuration.h>
#include <memory>
#include <mutex>

PortaudioWrapper::PortaudioWrapper() : m_callback(nullptr) {}

PortaudioWrapper::~PortaudioWrapper()
{
    stopRecording();
    if (m_stream)
    {
        Pa_CloseStream(m_stream);
    }
    Pa_Terminate();
}

bool PortaudioWrapper::initialize()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        Logger::logError("PortAudio initialization error: {}",
                         Pa_GetErrorText(err));
        return false;
    }

    // 检查默认输入设备
    const PaDeviceInfo *deviceInfo =
        Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    if (!deviceInfo)
    {
        Logger::logError("Failed to get default input device info.");
        return false;
    }

    // 从配置中读取配置参数
    auto &config = Configuration::getInstance();
    m_sampleRate =
        std::get<int32_t>(config.get("/audio/sample_rate", (int32_t)16000));
    m_sampleWidth =
        std::get<int32_t>(config.get("/audio/sample_width", (int32_t)2));
    m_framesPerBuffer = std::get<int32_t>(
        config.get("/audio/frames_per_buffer", (int32_t)3200));

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = m_channels;
    inputParams.sampleFormat = paInt16; // 16位PCM
    inputParams.suggestedLatency =
        Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&m_stream, &inputParams,
                        nullptr, // 不输出
                        m_sampleRate, m_framesPerBuffer, paClipOff,
                        &PortaudioWrapper::paCallback, this);
    if (err != paNoError)
    {
        Logger::logError("PortAudio open stream error: {}",
                         Pa_GetErrorText(err));
        return false;
    }

    return true;
}

PortaudioWrapper &PortaudioWrapper::setSampleRate(int sampleRate)
{
    m_sampleRate = sampleRate;
    return *this;
}

PortaudioWrapper &PortaudioWrapper::setSampleWidth(int sampleWidth)
{
    m_sampleWidth = sampleWidth;
    return *this;
}

PortaudioWrapper &PortaudioWrapper::setFramePerBuffer(int framesPerBuffer)
{
    m_framesPerBuffer = framesPerBuffer;
    return *this;
}

PortaudioWrapper &PortaudioWrapper::setChannels(int channels)
{
    m_channels = channels;
    return *this;
}

bool PortaudioWrapper::startRecording()
{
    if (m_isRecording)
    {
        return false;
    }
    if (!tryLockRecordingState())
    {
        return false;
    }

    clearRecordedData();
    PaError err = Pa_StartStream(m_stream);
    if (err == paNoError)
    {
        m_isRecording = true;
        unlockRecordingState();
        return true;
    }
    Logger::logError("PortAudio start stream error: {}", Pa_GetErrorText(err));
    unlockRecordingState();
    return false;
}

bool PortaudioWrapper::stopRecording()
{
    if (!m_isRecording || !m_stream)
    {
        return false;
    }
    if (!tryLockRecordingState())
    {
        return false;
    }

    PaError err = Pa_StopStream(m_stream);
    if (err == paNoError)
    {
        m_isRecording = false;
        unlockRecordingState();
        return true;
    }
    Logger::logError("PortAudio stop stream error {}", Pa_GetErrorText(err));
    unlockRecordingState();
    return false;
}

bool PortaudioWrapper::isRecording() const { return m_isRecording; }

std::vector<int16_t> PortaudioWrapper::getRecordedData() const
{
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_recordedData;
}

void PortaudioWrapper::clearRecordedData()
{
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_recordedData.clear();
}

int PortaudioWrapper::paCallback(const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo *timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData)
{

    auto *wrapper = static_cast<PortaudioWrapper *>(userData);
    if (!wrapper->isRecording())
    {
        return paComplete;
    }

    const int16_t *input = static_cast<const int16_t *>(inputBuffer);
    std::lock_guard<std::mutex> lock(wrapper->m_dataMutex);

    // 保存录制数据
    wrapper->m_recordedData
        .clear(); // 先清空数据，因为必要数据已经通过回调传递出去了
    wrapper->m_recordedData.insert(wrapper->m_recordedData.end(), input,
                                   input +
                                       framesPerBuffer * wrapper->m_channels);
    std::shared_ptr<PortaudioWrapper::Callback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(wrapper->m_mutForChangeCallback);
        callback = wrapper->m_callback;
    }
    if (callback)
    {
        wrapper->m_callback->onDataReady(
            wrapper->m_recordedData, wrapper->m_sampleRate, framesPerBuffer);
    }

    return paContinue;
}

bool PortaudioWrapper::tryLockRecordingState()
{
    return m_mutForChangeRecordingState.try_lock();
}

bool PortaudioWrapper::unlockRecordingState()
{
    m_mutForChangeRecordingState.unlock();
    return true;
}

std::vector<uint8_t> PortaudioWrapper::generateWavHeader(int sampleRate,
                                                         int channels,
                                                         int bitsPerSample,
                                                         size_t dataSize)
{

    std::vector<uint8_t> header(44);
    size_t byteRate = sampleRate * channels * bitsPerSample / 8;
    size_t blockAlign = channels * bitsPerSample / 8;

    // RIFF chunk
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    uint32_t chunkSize = 36 + dataSize;
    memcpy(&header[4], &chunkSize, 4);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';

    // fmt subchunk
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    uint32_t fmtSize = 16;
    memcpy(&header[16], &fmtSize, 4);
    uint16_t audioFormat = 1; // PCM
    memcpy(&header[20], &audioFormat, 2);
    memcpy(&header[22], &channels, 2);
    memcpy(&header[24], &sampleRate, 4);
    memcpy(&header[28], &byteRate, 4);
    memcpy(&header[32], &blockAlign, 2);
    memcpy(&header[34], &bitsPerSample, 2);

    // data subchunk
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    memcpy(&header[40], &dataSize, 4);

    return header;
}

void PortaudioWrapper::setCallback(Callback::Ptr callback)
{
    std::unique_lock<std::mutex> lk(m_mutForChangeCallback);
    m_callback = callback;
}
