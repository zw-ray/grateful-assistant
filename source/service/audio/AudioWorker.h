/*******************************************************************************
**     FileName: AudioWorker.h
**    ClassName: AudioWorker
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 16:12
**  Description:
*******************************************************************************/

#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include "PortaudioWrapper.h"
#include "VADDetector.h"
#include "kernel/EventBus.h"
#include "kernel/Events.h"
#include <atomic>
#include <map>
#include <memory>
#include <thread>

class AudioWorker
{
public:
    AudioWorker(std::shared_ptr<PortaudioWrapper> audioSampler,
                std::shared_ptr<VADDetector> vadDetector);
    ~AudioWorker();

    bool start();
    void stop();
    bool isRunning() const;

    enum class WorkerState
    {
        kPending,       // 等待检测结果状态
        kAwaitWakeword, // 等待唤醒词状态
        kAwaitContent,  // 等待内容状态
    };

    static std::string getWorkerStateString(WorkerState state);

    void setState(WorkerState state);

protected:
    void handleValidWakeWord(const AIEvents::ValidWakeWordEvent &event);
    void handleSpeechRecognitionResultReady(
        const AIEvents::SpeechRecognitionResultReadyEvent &event);

protected:
    std::atomic_bool m_isRunning;
    std::atomic_bool m_stop;
    std::shared_ptr<PortaudioWrapper> m_audioSampler;
    std::shared_ptr<VADDetector> m_vadDetector;
    std::thread m_workerThread;

    Subscription m_validWakeWordSubscription;
    Subscription m_speechRecognitionResultReadySubscription;

    // 当前状态：0 - 等待检测结果状态；1 - 等待唤醒词状态；2 - 等待内容状态；
    std::atomic<WorkerState> m_state;
    std::map<WorkerState, std::shared_ptr<PortaudioWrapper::Callback>>
        m_stateCallbacks;
}; // class AudioWorker

#endif // AUDIOWORKER_H
