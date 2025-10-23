#include "AudioWorker.h"

#include "PortaudioWrapper.h"
#include "VADDetector.h"
#include "kernel/Configuration.h"
#include "kernel/EventBus.h"
#include "kernel/Events.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <kernel/Logger.h>
#include <memory>
#include <vector>

class AudioPendingCallback : public PortaudioWrapper::Callback
{
public:
    AudioPendingCallback(AudioWorker &worker) : m_worker(worker)
    {
        auto &config = Configuration::getInstance();
        int32_t pendingWaitTime_MS = std::get<int32_t>(
            config.get("audio.pending_wait_time_ms", (int32_t)5000));
        m_maxActiveAudioCount = std::ceil(pendingWaitTime_MS / 200.0);
    }

    void onDataReady(const std::vector<int16_t> &data, int sampleRate,
                     int samplesPerBuffer) override
    {
        // 等待检测结果状态，直接忽略数据
        // 如果 1 秒后还没收到AI服务的返回信息，切换到等待唤醒词状态
        if (m_waitedAudioCount++ > m_maxActiveAudioCount)
        {
            m_worker.setState(AudioWorker::WorkerState::kAwaitWakeword);
            m_waitedAudioCount = 0;
        }
        Logger::logDebug("AudioPendingCallback: onDataReady, waitedAudioCount: "
                         "{}, maxActiveAudioCount: {}",
                         m_waitedAudioCount, m_maxActiveAudioCount);
    }

    void reset() override { m_waitedAudioCount = 0; }

protected:
    AudioWorker &m_worker;
    uint32_t m_waitedAudioCount = 0;
    uint32_t m_maxActiveAudioCount = 0;
};

class WakeWordDetectCallback : public PortaudioWrapper::Callback
{
public:
    WakeWordDetectCallback(std::shared_ptr<VADDetector> vadDetector,
                           AudioWorker &worker)
        : m_vadDetector(vadDetector), m_worker(worker)
    {
        auto &config = Configuration::getInstance();
        // 唤醒词检测最大时间间隔，默认5秒
#ifdef GA_DEBUG
        // 调试模式下，默认10秒
        const int32_t kDefaultWakeWordMaxTime_MS = 10000;
#else
        // 非调试模式下，默认5秒
        const int32_t kDefaultWakeWordMaxTime_MS = 5000;
#endif
        int32_t wakeWordMaxTime_MS = std::get<int32_t>(
            config.get("/audio/wake_word_detect/wake_word_max_time_ms",
                       (int32_t)kDefaultWakeWordMaxTime_MS));
        m_maxActiveAudioCount = std::ceil(wakeWordMaxTime_MS / 200.0);
        // 唤醒词检测后静音时长间隔，默认1秒
        int32_t inactiveAudioMaxTime_MS = std::get<int32_t>(
            config.get("/audio/wake_word_detect/inactive_audio_max_time_ms",
                       (int32_t)1000));

        m_maxInactiveAudioCount = std::ceil(inactiveAudioMaxTime_MS / 200.0);
    }

    void onDataReady(const std::vector<int16_t> &data, int sampleRate,
                     int samplesPerBuffer) override
    {
        bool isAudioActive = false;
        if (m_vadDetector)
        {
            isAudioActive = m_vadDetector->isActiveAudio(data);
        }
        if (isAudioActive)
        {
            m_activeAudioCount++;
            m_data.insert(m_data.end(), data.begin(), data.end());
        }
        else
        {
            m_inactiveAudioCount++;
        }

        Logger::logDebug(
            "WakeWordDetectCallback: onDataReady, activeAudioCount: "
            "{}, inactiveAudioCount: {}, maxActiveAudioCount: {}, "
            "maxInactiveAudioCount: {}",
            m_activeAudioCount.load(), m_inactiveAudioCount.load(),
            m_maxActiveAudioCount, m_maxInactiveAudioCount);

        if (m_activeAudioCount == 0 &&
            m_inactiveAudioCount >= m_maxInactiveAudioCount)
        {
            // 静音时长超过阈值，认为需要重置状态
            reset();
            return;
        }

        if (m_activeAudioCount >= m_maxActiveAudioCount ||
            (m_inactiveAudioCount >= m_maxInactiveAudioCount &&
             m_activeAudioCount > 0))
        {
            // 两种情况需要发送数据
            // 1. 有声音之后，静音时长超过阈值
            // 2. 持续有声音，但是有声时长超限
            sendValidWakeWordEvent();
            reset();
        }
    }

    void reset() override
    {
        m_activeAudioCount = 0;
        m_inactiveAudioCount = 0;
        m_data.clear();
    }

protected:
    void sendValidWakeWordEvent()
    {
        AudioEvents::CheckIsWakewordEvent event;
        event.audioData = m_data;
        event.time = std::time(nullptr);
        // 向ai服务发送事件
        EventBus::getInstance()
            .publish_async<AudioEvents::CheckIsWakewordEvent>(event);
        Logger::logDebug("AudioService: WakeWordDetectCallback send event of "
                         "AudioEvents::CheckIsWakewordEvent");

        // change worker state to pending
        m_worker.setState(AudioWorker::WorkerState::kPending);
    }

protected:
    std::shared_ptr<VADDetector> m_vadDetector;
    std::vector<int16_t> m_data;
    std::atomic_int16_t m_activeAudioCount{0};
    std::atomic_int16_t m_inactiveAudioCount{0};
    uint32_t m_maxActiveAudioCount{0};
    uint32_t m_maxInactiveAudioCount{0};
    AudioWorker &m_worker;
};

class ContentRecognitionCallback : public PortaudioWrapper::Callback
{
public:
    ContentRecognitionCallback(std::shared_ptr<VADDetector> vadDetector,
                               AudioWorker &worker)
        : m_worker(worker), m_vadDetector(vadDetector)
    {
        auto &config = Configuration::getInstance();
        // 内容识别最大静音时长间隔，默认3秒，超过该时间间隔没有活动音频，认为需要重置状态
        std::string key =
            "/audio/content_recognition/start_inactive_audio_max_time_ms";
        int32_t startInactiveAudioMaxTime_MS =
            std::get<int32_t>(config.get(key, (int32_t)3000));
        m_maxStartInactiveAudioCount =
            std::ceil(startInactiveAudioMaxTime_MS / 200.0);

        // 内容识别后静音时长间隔，默认3秒
        key = "/audio/content_recognition/stop_inactive_audio_max_time_ms";
        int32_t stopInactiveAudioMaxTime_MS =
            std::get<int32_t>(config.get(key, (int32_t)3000));
        m_maxStopInactiveAudioCount =
            std::ceil(stopInactiveAudioMaxTime_MS / 200.0);
    }

    void onDataReady(const std::vector<int16_t> &data, int sampleRate,
                     int samplesPerBuffer) override
    {
        // 检测是否存在静音，如果静音了，那么就将数据发送给AI服务

        bool isAudioActive = true;
        if (m_vadDetector)
        {
            isAudioActive = m_vadDetector->isActiveAudio(data);
        }
        m_data.insert(m_data.end(), data.begin(), data.end());
        if (!isAudioActive)
        {
            m_inactiveAudioCount++;
        }
        else
        {
            m_maxStartInactiveAudioCount = 0;
            m_inactiveAudioCount = 0;
            m_activeAudioCount++;
        }
        Logger::logDebug(
            "ContentRecognitionCallback: onDataReady, activeAudioCount: "
            "{}, inactiveAudioCount: {}, maxStartInactiveAudioCount: {}, "
            "maxStopInactiveAudioCount: {}",
            m_activeAudioCount.load(), m_inactiveAudioCount.load(),
            m_maxStartInactiveAudioCount, m_maxStopInactiveAudioCount);
        if (m_inactiveAudioCount >= m_maxStartInactiveAudioCount &&
            m_activeAudioCount == 0)
        {
            // 静音时长超过阈值并且没有活动音频，认为需要重置状态
            m_worker.setState(AudioWorker::WorkerState::kAwaitWakeword);
            // 并发送AudioEvents::AudioContentRecordingDoneEvent事件
            {
                AudioEvents::AudioContentRecordingDoneEvent event;
                event.audioData = m_data;
                event.time = std::time(nullptr);
                // 向ai服务发送事件
                EventBus::getInstance()
                    .publish_async<AudioEvents::AudioContentRecordingDoneEvent>(
                        event);
            }
            reset(); // 重置状态
            Logger::logDebug("ContentRecognitionCallback: reset, "
                             "inactiveAudioCount: "
                             "{}",
                             m_inactiveAudioCount.load());
            return;
        }
        if (m_activeAudioCount > 0 &&
            m_inactiveAudioCount >= m_maxStopInactiveAudioCount)
        {
            // 活动音频后静音时间超过阈值，认为需要发送数据
            handleRecordingReady();
            reset();
        }
    }

    void reset() override
    {
        m_inactiveAudioCount = 0;
        m_data.clear();
    }

protected:
    void handleRecordingReady()
    {
        Logger::logDebug("ContentRecognitionCallback: handleRecordingReady, "
                         "inactiveAudioCount: "
                         "{}",
                         m_inactiveAudioCount.load());
        AudioEvents::AudioContentRecordingDoneEvent event;
        event.audioData = m_data;
        event.time = std::time(nullptr);
        // 向ai服务发送事件
        EventBus::getInstance()
            .publish_async<AudioEvents::AudioContentRecordingDoneEvent>(event);
        Logger::logDebug(
            "AudioService: ContentRecognitionCallback send event of "
            "AudioEvents::AudioContentRecordingDoneEvent, with data size: {}",
            m_data.size());

        m_worker.setState(AudioWorker::WorkerState::kPending);
        reset();
    }

protected:
    std::shared_ptr<VADDetector> m_vadDetector;
    AudioWorker &m_worker;
    std::vector<int16_t> m_data;
    std::atomic_int32_t m_inactiveAudioCount{0};
    int32_t m_maxStartInactiveAudioCount{0}; // 起始检测最大静音时长间隔
    int32_t m_maxStopInactiveAudioCount{0};  // 结束检测最大静音时长间隔
    std::atomic_int32_t m_activeAudioCount{0};
};

class SaveToFileCallback : public PortaudioWrapper::Callback
{
    static uint32_t nameSuffix;

public:
    void onDataReady(const std::vector<int16_t> &data, int sampleRate,
                     int samplesPerBuffer) override
    {
        // 保存音频数据到文件
        m_data.insert(m_data.end(), data.begin(), data.end());
        m_count++;
        if (m_count >= 100) // 20s
        {
            auto header = PortaudioWrapper::generateWavHeader(
                16000, 1, 16, sizeof(int16_t) * m_data.size());
            std::string fileName = fmt::format("output_{}.wav", nameSuffix++);
            std::ofstream file(fileName, std::ios::binary);
            file.write(reinterpret_cast<const char *>(header.data()),
                       header.size());
            file.write(reinterpret_cast<const char *>(m_data.data()),
                       m_data.size() * sizeof(int16_t));
            file.close();
            m_data.clear();
            m_count = 0;

            Logger::logInfo("Audio data has been written into: {}", fileName);
        }
    }

    void reset() override
    {
        m_data.clear();
        m_count = 0;
    }

protected:
    std::vector<int16_t> m_data;
    uint32_t m_count;
};

uint32_t SaveToFileCallback::nameSuffix = 0;

AudioWorker::AudioWorker(std::shared_ptr<PortaudioWrapper> audioSampler,
                         std::shared_ptr<VADDetector> vadDetector)
    : m_vadDetector(vadDetector), m_audioSampler(audioSampler),
      m_isRunning(false), m_stop(false), m_validWakeWordSubscription([]() {}),
      m_speechRecognitionResultReadySubscription([]() {}),
      m_state(WorkerState::kAwaitWakeword)
{
    m_stateCallbacks[WorkerState::kPending] =
        std::make_shared<AudioPendingCallback>(*this);
    m_stateCallbacks[WorkerState::kAwaitWakeword] =
        std::make_shared<WakeWordDetectCallback>(m_vadDetector, *this);
    m_stateCallbacks[WorkerState::kAwaitContent] =
        std::make_shared<ContentRecognitionCallback>(m_vadDetector, *this);
    m_audioSampler->setCallback(m_stateCallbacks[m_state]);

    auto func = std::bind(&AudioWorker::handleValidWakeWord, this,
                          std::placeholders::_1);
    m_validWakeWordSubscription =
        EventBus::getInstance().on<AIEvents::ValidWakeWordEvent>(func);
    auto func2 = std::bind(&AudioWorker::handleSpeechRecognitionResultReady,
                           this, std::placeholders::_1);
    m_speechRecognitionResultReadySubscription =
        EventBus::getInstance().on<AIEvents::SpeechRecognitionResultReadyEvent>(
            func2);
}

AudioWorker::~AudioWorker() { stop(); }

bool AudioWorker::start()
{
    if (m_isRunning.load())
    {
        return true;
    }

    if (m_audioSampler == nullptr || m_vadDetector == nullptr)
    {
        Logger::logError("AudioWorker: audioSampler or vadDetector is nullptr");
        return false;
    }

    auto func = [&]()
    {
        auto pendingCallback = std::make_shared<AudioPendingCallback>(*this);
        auto awaitWakeWordCallback =
            std::make_shared<WakeWordDetectCallback>(m_vadDetector, *this);
        auto awaitContentCallback =
            std::make_shared<ContentRecognitionCallback>(m_vadDetector, *this);
        auto saveToFileCallback = std::make_shared<SaveToFileCallback>();
        auto pendingStartTime = std::chrono::system_clock::now();
        while (!m_stop.load())
        {
            switch (m_state)
            {
            case WorkerState::kPending:
            {
                auto now = std::chrono::system_clock::now();
                if (m_audioSampler->isRecording())
                {
                    m_audioSampler->stopRecording();
                    pendingStartTime = now;
                }
#ifdef GA_DEBUG
                const int maxWaitingSeconds = 20;
#else
                const int maxWaitingSeconds = 5;
#endif
                if (std::chrono::duration_cast<std::chrono::seconds>(
                        now - pendingStartTime)
                        .count() >= maxWaitingSeconds)
                {
                    setState(WorkerState::kAwaitWakeword);
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            case WorkerState::kAwaitWakeword:
            {
                m_audioSampler->setCallback(awaitWakeWordCallback);
            }
            break;
            case WorkerState::kAwaitContent:
            {
                m_audioSampler->setCallback(awaitContentCallback);
            }
            break;
            default:
            {
                m_audioSampler->setCallback(saveToFileCallback);
            }
            break;
            }

            if (m_state != WorkerState::kPending)
            {
                if (!m_audioSampler->isRecording())
                {
                    m_audioSampler->startRecording();
                }
            }
        }
        if (m_audioSampler->isRecording())
        {
            m_audioSampler->stopRecording();
        }
        m_isRunning = false;
    };
    m_workerThread = std::thread(func);
    m_isRunning = true;
    return true;
}

void AudioWorker::stop()
{
    m_validWakeWordSubscription.unsubscribe();
    m_speechRecognitionResultReadySubscription.unsubscribe();
    m_stop = true;
    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
}

bool AudioWorker::isRunning() const { return m_isRunning; }

void AudioWorker::handleValidWakeWord(const AIEvents::ValidWakeWordEvent &event)
{
    Logger::logDebug("AudioWorker: handleValidWakeWord, change state to "
                     "kAwaitContent, current state is {}",
                     getWorkerStateString(m_state.load()));
    // 发送系统消息，提示用户可以开始说话
    SystemEvents::SystemMessageEvent systemMessageEvent;
    systemMessageEvent.time =
        std::chrono::system_clock::now().time_since_epoch().count();
    systemMessageEvent.message = u8"唤醒词校验成功，您可以开始说话了";
    EventBus::getInstance().publish_async<SystemEvents::SystemMessageEvent>(
        systemMessageEvent);
    Logger::logDebug(u8"唤醒词校验成功，您可以开始说话了");
    setState(WorkerState::kAwaitContent);
}

void AudioWorker::handleSpeechRecognitionResultReady(
    const AIEvents::SpeechRecognitionResultReadyEvent &event)
{
    setState(WorkerState::kAwaitWakeword);
}

std::string AudioWorker::getWorkerStateString(WorkerState state)
{
    switch (state)
    {
    case WorkerState::kPending:
        return "kPending";
    case WorkerState::kAwaitWakeword:
        return "kAwaitWakeword";
    case WorkerState::kAwaitContent:
        return "kAwaitContent";
    default:
        return "Unknown";
    }
}

void AudioWorker::setState(WorkerState state)
{
    Logger::logDebug("AudioWorker: current state is {}, setState to {}",
                     getWorkerStateString(m_state.load()),
                     getWorkerStateString(state));
    m_stateCallbacks[m_state]->reset();
    m_state.store(state);
}
