#include "kernel/DynamicLinker.h"
#include <ai/Model.h>
#include <ai/Provider.h>
#include <ai/ProviderManager.h>
#include <kernel/Configuration.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <kernel/Logger.h>
#include <memory>
#include <vector>

namespace ai {

static constexpr std::string_view kDefaultProviderName = "SiliconFlow";

struct ProviderManager::Data
{
    std::map<std::string, std::shared_ptr<Provider>> providers;
    std::string currentProviderName;

    void parseModels(Provider::Ptr provider, const std::string &providerKey)
    {
        auto &config = Configuration::getInstance();
        uint32_t modelCount = config.arraySize(providerKey);
        for (uint32_t i = 0; i < modelCount; ++i)
        {
            std::string modelName =
                std::get<std::string>(config.get(fmt::format("{}/{}/name", providerKey, i), ""));
            if (modelName == "")
            {
                continue;
            }
            auto model = provider->createModel(modelName);
            if (!model)
            {
                continue;
            }
            // parameters
            Model::ModelParams params = model->getParams();
            params.maxTokens = std::get<int32_t>(
                config.get(fmt::format("{}/{}/parameters/max_tokens", providerKey, i), 2048));
            params.temperature = std::get<double>(
                config.get(fmt::format("{}/{}/parameters/temperature", providerKey, i), 0.7));
            params.topP = std::get<double>(
                config.get(fmt::format("{}/{}/parameters/top_p", providerKey, i), 0.9));
            params.topK = std::get<int32_t>(
                config.get(fmt::format("{}/{}/parameters/top_k", providerKey, i), 40));
            params.repetitionPenalty = std::get<double>(config.get(
                fmt::format("{}/{}/parameters/repetition_penalty", providerKey, i), 1.0f));
            params.enableThinking = std::get<bool>(
                config.get(fmt::format("{}/{}/parameters/enable_thinking", providerKey, i), false));
            params.thinkingBudget = std::get<int32_t>(
                config.get(fmt::format("{}/{}/parameters/thinking_budget", providerKey, i), 1000));
            params.enableStreaming = std::get<bool>(config.get(
                fmt::format("{}/{}/parameters/enable_streaming", providerKey, i), false));

            model->setParams(params);
        }
    }

    std::vector<Provider::Ptr> parseProviders()
    {
        std::vector<Provider::Ptr> ret;
        auto &config = Configuration::getInstance();
        uint32_t providerCount = config.arraySize("/ai/providers");
        for (uint32_t i = 0; i < providerCount; ++i)
        {
            std::string providerName =
                std::get<std::string>(config.get(fmt::format("/ai/providers/{}/name", i), ""));
            if (providerName == "")
            {
                continue;
            }
            std::shared_ptr<Provider> provider = std::make_shared<Provider>();
            if (this->providers.find(providerName) != this->providers.end())
            {
                provider = this->providers[providerName];
            }
            provider->m_name = providerName;
            const std::string baseUrl =
                std::get<std::string>(config.get(fmt::format("/ai/providers/{}/base_url", i), ""));
            provider->setBaseUrl(baseUrl);
            const std::string apiKey =
                std::get<std::string>(config.get(fmt::format("/ai/providers/{}/api_key", i), ""));
            provider->setApiKey(apiKey);

            parseModels(provider, fmt::format("/ai/providers/{}/models", i));

            ret.push_back(provider);
        }
        return ret;
    }
};

ProviderManager::ProviderManager() : m_data(std::make_unique<Data>()) {}
ProviderManager::~ProviderManager() {}

void ProviderManager::initialize()
{
    // try to load from providers extension
    try
    {
        auto ext = DynamicLinker::getInstance().loadExtension("providers");
        if (!ext)
        {
            Logger::logError("Failed to load providers extension.");
        }
        ext->initialize();
    }
    catch (const std::exception &e)
    {
        Logger::logError("Failed to load providers extension: {}", e.what());
    }
    auto providers = m_data->parseProviders();
    for (auto &provider : providers)
    {
        m_data->providers[provider->getName()] = provider;
    }
    auto &config = Configuration::getInstance();
    auto activeProvider = std::get<std::string>(
        config.get("/ai/active_provider", Configuration::ConfigValueType(std::string(""))));
    if (activeProvider != "")
    {
        m_data->currentProviderName = activeProvider;
    }
}

std::shared_ptr<Provider> ProviderManager::getCurrentProvider() const
{
    if (m_data->currentProviderName == "")
    {
        Logger::logWarning("No provider selected. Using default provider: {}",
                           kDefaultProviderName);
        m_data->currentProviderName = kDefaultProviderName;
    }
    std::string providerName = m_data->currentProviderName;
    auto iter = m_data->providers.find(providerName);
    if (iter == m_data->providers.end())
    {
        Logger::logError("Provider {} not found.", providerName);
        return nullptr;
    }
    return iter->second;
}

std::vector<std::shared_ptr<Provider>> ProviderManager::getRegisteredProviders() const
{
    std::vector<std::shared_ptr<Provider>> providers;
    for (auto &pair : m_data->providers)
    {
        providers.push_back(pair.second);
    }
    return providers;
}

void ProviderManager::setCurrentProvider(const std::string &providerName)
{
    if (m_data->providers.find(providerName) == m_data->providers.end())
    {
        Logger::logError("Provider {} not found.", providerName);
        return;
    }
    auto &config = Configuration::getInstance();
    config.set("/ai/active_provider", providerName);
    Logger::logDebug("Current provider set to: {}", providerName);
}

std::shared_ptr<Provider> ProviderManager::getProvider(const std::string &providerName) const
{
    auto iter = m_data->providers.find(providerName);
    if (iter == m_data->providers.end())
    {
        Logger::logError("Provider {} not found.", providerName);
        return nullptr;
    }
    return iter->second;
}

void ProviderManager::registerProvider(std::shared_ptr<Provider> provider)
{
    auto iter = m_data->providers.find(provider->getName());
    if (iter != m_data->providers.end())
    {
        Logger::logError("Provider {} already registered.", provider->getName());
        return;
    }
    m_data->providers[provider->getName()] = provider;
    if (provider->serializable())
    {
        auto &config = Configuration::getInstance();
        uint32_t providerCount = config.arraySize("/ai/providers");
        config.push_back(fmt::format("/ai/providers"));
        const std::string baseKey = fmt::format("/ai/providers/{}", providerCount);
        config.set(fmt::format("{}/name", baseKey), provider->getName());
        config.set(fmt::format("{}/base_url", baseKey), provider->getBaseUrl());
        config.set(fmt::format("{}/api_key", baseKey), provider->getApiKey());

        auto models = provider->getModels();
        int idx = 0;
        for (auto model : models)
        {
            config.push_back(fmt::format("{}/models", baseKey));
            const std::string baseModelKey = fmt::format("{}/models/{}", baseKey, idx++);
            config.set(fmt::format("{}/name", baseModelKey), model->getModelName());
            const auto params = model->getParams();
            config.set(fmt::format("{}/parameters/max_tokens", baseModelKey), params.maxTokens);
            config.set(fmt::format("{}/parameters/temperature", baseModelKey), params.temperature);
            config.set(fmt::format("{}/parameters/top_p", baseModelKey), params.topP);
            config.set(fmt::format("{}/parameters/top_k", baseModelKey), params.topK);
            config.set(fmt::format("{}/parameters/repetition_penalty", baseModelKey),
                       params.repetitionPenalty);
            config.set(fmt::format("{}/parameters/enable_thinking", baseModelKey),
                       params.enableThinking);
            config.set(fmt::format("{}/parameters/thinking_budget", baseModelKey),
                       params.thinkingBudget);
            config.set(fmt::format("{}/parameters/enable_streaming", baseModelKey),
                       params.enableStreaming);
        }
    }
    Logger::logDebug("Provider {} registered.", provider->getName());
}

bool ProviderManager::isProviderRegistered(const std::string &providerName) const
{
    return m_data->providers.find(providerName) != m_data->providers.end();
}

void ProviderManager::unregisterProvider(const std::string &providerName)
{
    if (m_data->providers.find(providerName) == m_data->providers.end())
    {
        Logger::logError("Provider {} not found.", providerName);
        return;
    }
    m_data->providers.erase(providerName);
    Logger::logDebug("Provider {} unregistered.", providerName);
}

} // namespace ai
