#include "ProvidersExtension.h"
#include <ai/AI.h>
#include <ai/ProviderManager.h>
#include <kernel/Extension.h>

#include "OllamaProvider.h"
#include "SiliconFlowProvider.h"

ProvidersExtension::ProvidersExtension()
{
    m_info.apiVersion = 1;
    m_info.name = "ProvidersExtension";
    m_info.type = "ext";
    m_info.path = "providers.ext";
    m_info.detail.author = "Geocat & LittleBottle";
    m_info.detail.version = "1.0.0";
    m_info.detail.description = "Provides more AI providers for GrateAssistant.";
    m_info.detail.license = "MIT";
    m_info.detail.platforms = Extension::ExtensionInfo::Detail::kWindowsX64 |
                              Extension::ExtensionInfo::Detail::kLinuxX64 |
                              Extension::ExtensionInfo::Detail::kMacOSX64;
    m_info.detail.url = "https://github.com/Geocat/ProvidersExtension";
    m_info.detail.contact = "geocat@qq.com";
    m_info.detail.keywords = {"Providers"};
    m_info.detail.category = Extension::ExtensionInfo::Detail::kAI;
}

ProvidersExtension::~ProvidersExtension() {}

void ProvidersExtension::initialize()
{
    // 注册 AI 提供程序
    auto providerManager = ai::AI::getInstance().getProviderManager();
    m_siliconFlowProvider = std::make_shared<SiliconFlowProvider>();
    providerManager->registerProvider(m_siliconFlowProvider);
    m_ollamaProvider = std::make_shared<OllamaProvider>();
    providerManager->registerProvider(m_ollamaProvider);
}

void ProvidersExtension::finalize()
{
    // 注销 AI 提供程序
    auto providerManager = ai::AI::getInstance().getProviderManager();
    providerManager->unregisterProvider(m_siliconFlowProvider->getName());
    providerManager->unregisterProvider(m_ollamaProvider->getName());
}

std::string ProvidersExtension::name() const { return m_info.name; }

Extension::ExtensionInfo ProvidersExtension::info() const { return m_info; }

extern "C" EXTENSION_EXPORT Extension *createExtension() { return new ProvidersExtension(); }

extern "C" EXTENSION_EXPORT void destroyExtension(Extension *extension) { delete extension; }
