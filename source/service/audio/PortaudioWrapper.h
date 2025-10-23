/*******************************************************************************
**     FileName: PortaudioWrapper.h
**    ClassName: PortaudioWrapper
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 20:10
**  Description:
*******************************************************************************/

// audio/PortaudioWrapper.h 扩展
#ifndef PORTAUDIOWRAPPER_H
#define PORTAUDIOWRAPPER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <portaudio.h>
#include <vector>

class PortaudioWrapper
{
public:
    PortaudioWrapper();
    ~PortaudioWrapper();

    bool initialize();
    PortaudioWrapper &setSampleRate(int sampleRate);
    PortaudioWrapper &setSampleWidth(int sampleWidth); // 单位：字节
    PortaudioWrapper &setFramePerBuffer(int framesPerBuffer);
    PortaudioWrapper &setChannels(int channels);

    bool startRecording();
    bool stopRecording();
    bool isRecording() const;
    std::vector<int16_t> getRecordedData() const; // 获取录制的PCM数据
    void clearRecordedData();

    // WAV格式头部生成
    static std::vector<uint8_t> generateWavHeader(int sampleRate, int channels,
                                                  int bitsPerSample,
                                                  size_t dataSize);

    class Callback
    {
    public:
        using Ptr = std::shared_ptr<Callback>;
        virtual ~Callback() = default;
        virtual void onDataReady(const std::vector<int16_t> &data,
                                 int sampleRate, int samplesPerBuffer) = 0;
        virtual void reset() = 0;
    };
    void setCallback(Callback::Ptr callback);

protected:
    bool tryLockRecordingState();
    bool unlockRecordingState();
    static int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData);

private:
    PaStream *m_stream = nullptr;
    int m_sampleRate = 16000;     // 硅基API推荐16000Hz
    int m_sampleWidth = 2;        // 16位
    int m_framesPerBuffer = 3200; // 约200ms
    int m_channels = 1;           // 单声道
    bool m_isRecording = false;
    std::vector<int16_t> m_recordedData;
    mutable std::mutex m_dataMutex;

    std::mutex m_mutForChangeCallback;
    Callback::Ptr m_callback;
    std::mutex m_mutForChangeRecordingState;
};

#endif // PORTAUDIOWRAPPER_H
