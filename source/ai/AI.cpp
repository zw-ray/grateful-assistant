#include "ai/RoleManager.h"
#include "kernel/Configuration.h"
#include <ai/AI.h>
#include <ai/IntentManager.h>
#include <ai/Model.h>
#include <ai/Provider.h>
#include <ai/ProviderManager.h>
#include <atomic>
#include <chrono>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/Logger.h>
#include <memory>
#include <string>
#include <string_view>

namespace ai {

static constexpr std::string_view kDefaultAudioModelName = "FunAudioLLM/SenseVoiceSmall";
static constexpr std::string_view kDefaultTextModelName = "Qwen/Qwen3-8B";
static constexpr std::string_view kDefaultProviderName = "SiliconFlow";
static constexpr std::string_view kDefaultWakeWord = u8"小竹小竹";

struct AI::Data
{
    std::shared_ptr<ProviderManager> providerManager;
    std::shared_ptr<IntentManager> intentManager;
    std::shared_ptr<RoleManager> roleManager;

    static constexpr std::string_view kSystemRoleName = "SystemRole";
    static constexpr std::string_view kAwakeWordVerifyRole = "AwakeWordVerifyRole";

    Subscription checkWakeWordSubscription;             // 检查是否是唤醒词的订阅
    Subscription audioContentRecordingDoneSubscription; // 音频内容识别订阅

    Data() : checkWakeWordSubscription([]() {}), audioContentRecordingDoneSubscription([]() {})
    {
        providerManager = std::make_shared<ProviderManager>();
        intentManager = std::make_shared<IntentManager>();
        roleManager = std::make_shared<RoleManager>(*intentManager);
    }

    Provider::Ptr getValidProvider() const
    {
        auto &config = Configuration::getInstance();
        auto provider = providerManager->getCurrentProvider();
        if (!provider)
        {
            Logger::logDebug("getValidAudioModel: No provider selected. Using default "
                             "provider: {}",
                             kDefaultProviderName); // 调试日志信息
            Logger::logWarning("No provider selected. Using default provider: {}",
                               kDefaultProviderName);
            provider = providerManager->getProvider(std::string(kDefaultProviderName));
        }
        return provider;
    }

    Model::Ptr getValidAudioModel() const
    {
        auto provider = getValidProvider();
        if (!provider)
        {
            return nullptr;
        }
        auto &config = Configuration::getInstance();
        std::string audioModelName = std::get<std::string>(
            config.get("/ai/active_audio_model",
                       Configuration::ConfigValueType(std::string(kDefaultAudioModelName))));
        if (audioModelName == "")
        {
            Logger::logDebug("getValidAudioModel: No audio model selected. Using "
                             "default model: {}",
                             kDefaultAudioModelName);
            Logger::logWarning("No audio model selected. Using default model: {}",
                               kDefaultAudioModelName);
            audioModelName = kDefaultAudioModelName;
        }
        auto model = provider->createModel(audioModelName);
        if (!model)
        {
            Logger::logDebug("getValidAudioModel: Failed to create model {}", audioModelName);
            Logger::logError("Failed to create model {}", audioModelName);
            return nullptr;
        }
        return model;
    }

    Model::Ptr getValidTextModel() const
    {
        auto provider = getValidProvider();
        if (!provider)
        {
            return nullptr;
        }
        auto &config = Configuration::getInstance();
        std::string textModelName = std::get<std::string>(
            config.get("/ai/active_text_model",
                       Configuration::ConfigValueType(std::string(kDefaultTextModelName))));
        if (textModelName == "")
        {
            Logger::logDebug("getValidTextModel: No audio model selected. Using "
                             "default model: {}",
                             kDefaultTextModelName);
            Logger::logWarning("No audio model selected. Using default model: {}",
                               kDefaultTextModelName);
            textModelName = kDefaultTextModelName;
        }
        auto model = provider->createModel(textModelName);
        if (!model)
        {
            Logger::logDebug("AudigetValidTextModel2Text: Failed to create model {}",
                             textModelName);
            Logger::logError("Failed to create model {}", textModelName);
            return nullptr;
        }
        return model;
    }

    void checkIsWakeWord(const AudioEvents::CheckIsWakewordEvent &event) const
    {
        auto model = getValidAudioModel();
        if (!model)
        {
            Logger::logError("No valid audio model found.");
            return;
        }
        auto res = model->speech2Text(event.audioData);
        if (!res.isSuccess())
        {
            Logger::logError("Audio2Text: Failed to recognize audio: {}", res.error);
            return;
        }
        auto &config = Configuration::getInstance();
        std::string wakeWord = std::get<std::string>(config.get(
            "ai.wake_word", Configuration::ConfigValueType(std::string(kDefaultWakeWord))));
        if (wakeWord.empty())
        {
            Logger::logError("Audio2Text: No wake word configured. use default wake "
                             "word: {}",
                             kDefaultWakeWord);
            wakeWord = kDefaultWakeWord;
        }

        // 校验唤醒词
        auto verifyRole = roleManager->getRole(kAwakeWordVerifyRole.data());
        if (!verifyRole)
        {
            Logger::logError("No AwakeWordVerifyRole found.");
            return;
        }
        auto verifyRes = verifyRole->handleRequest(this->getValidTextModel(), res.response);
        if (verifyRes != "success")
        {
            Logger::logError("AwakeWordVerifyRole: Failed to verify wake word: {}", verifyRes);
            return;
        }

        // 唤醒词被检测到， 发送唤醒词有效事件
        {
            AIEvents::ValidWakeWordEvent event;
            event.time = std::chrono::system_clock::now().time_since_epoch().count();
            EventBus::getInstance().publish_async(event);
        }
    }

    void onAudioContentRecordingDone(const AudioEvents::AudioContentRecordingDoneEvent &event) const
    {
        auto model = getValidAudioModel();
        if (!model)
        {
            Logger::logError("No valid audio model found.");
            return;
        }
        auto res = model->speech2Text(event.audioData);
        if (!res.isSuccess())
        {
            Logger::logError("Audio2Text: Failed to recognize audio: {}", res.error);
            return;
        }
#ifdef GA_DEBUG
        // 发送系统消息，显示识别结果
        {
            SystemEvents::SystemMessageEvent messageEvent;
            messageEvent.time = std::time(nullptr);
            messageEvent.message =
                fmt::format("Audio content recognition result: {}", res.response);
            EventBus::getInstance().publish_async<SystemEvents::SystemMessageEvent>(messageEvent);
        }
#endif
        // 将识别结果发送到系统角色
        auto systemRole = roleManager->getRole(kSystemRoleName.data());
        if (!systemRole)
        {
            Logger::logError("No SystemRole found.");
            return;
        }
        auto systemRes = systemRole->handleRequest(this->getValidTextModel(), res.response);
        if (systemRes != "success")
        {
            Logger::logError("SystemRole: Failed to handle request: {}", systemRes);
            return;
        }
    }
};

AI::AI() : m_data(std::make_unique<Data>()) {}

AI::~AI() {}

AI &AI::getInstance()
{
    static AI instance;
    return instance;
}

bool AI::initialize()
{
    try
    {
        Logger::logInfo("AI Service initialized successfully");
        m_data->providerManager->initialize();
        m_data->intentManager->initialize();
        m_data->roleManager->initialize();

        auto func = std::bind(&AI::Data::checkIsWakeWord, m_data.get(), std::placeholders::_1);
        m_data->checkWakeWordSubscription =
            EventBus::getInstance().on<AudioEvents::CheckIsWakewordEvent>(func);
        m_data->audioContentRecordingDoneSubscription =
            EventBus::getInstance().on<AudioEvents::AudioContentRecordingDoneEvent>(std::bind(
                &AI::Data::onAudioContentRecordingDone, m_data.get(), std::placeholders::_1));

        return true;
    }
    catch (const std::exception &e)
    {
        Logger::logError("Failed to initialize AI Service: {}", e.what());
        return false;
    }
}

std::shared_ptr<RoleManager> AI::getRoleManager() const { return m_data->roleManager; }

std::shared_ptr<ProviderManager> AI::getProviderManager() const { return m_data->providerManager; }

std::shared_ptr<IntentManager> AI::getIntentManager() const { return m_data->intentManager; }

} // namespace ai
