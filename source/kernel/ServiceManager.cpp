#include <kernel/ServiceManager.h>

#include <memory>
#include <string>
#include <vector>

#include <kernel/Configuration.h>
#include <kernel/DynamicLinker.h>
#include <kernel/IService.h>
#include <kernel/Logger.h>

struct ServiceManager::Data
{
    std::vector<std::shared_ptr<IService>> loadedServices;
};

ServiceManager::ServiceManager() : m_data(std::make_unique<Data>()) {}

ServiceManager::~ServiceManager() {}

ServiceManager &ServiceManager::getInstance()
{
    static ServiceManager instance;
    return instance;
}

std::vector<std::shared_ptr<IService>> ServiceManager::loadSystemServices()
{
    auto &config = Configuration::getInstance();
    auto value =
        config.get("/app/system_services",
                   Configuration::ConfigValueType(std::vector<std::string>()));
    std::vector<std::string> systemServices =
        std::get<std::vector<std::string>>(value);
    std::vector<std::shared_ptr<IService>> loadedServices;
    DynamicLinker &linker = DynamicLinker::getInstance();
    for (const auto &serviceName : systemServices)
    {
        auto service = linker.loadService(serviceName);
        if (service)
        {
            if (service->initialize())
            {
                loadedServices.push_back(service);
            }
            else
            {
                Logger::logError("Failed to initialize service: {}",
                                 serviceName);
            }
        }
        else
        {
            Logger::logError("Failed to load service: {}", serviceName);
        }
    }
    m_data->loadedServices = loadedServices;
    return loadedServices;
}

void ServiceManager::unloadSystemServices()
{
    for (auto service : m_data->loadedServices)
    {
        if (service->isInitialized() && service->isRunning())
        {
            service->shutdown();
        }
    }
}
