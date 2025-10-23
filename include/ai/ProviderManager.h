/*******************************************************************************
**     FileName: ProviderManager.h
**    ClassName: ProviderManager
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/09 15:38
**  Description:
*******************************************************************************/

#ifndef PROVIDERMANAGER_H
#define PROVIDERMANAGER_H

#include <ai/AIExport.h>
#include <ai/Provider.h>
#include <memory>
#include <vector>

namespace ai {

class AI_API ProviderManager
{
public:
    ProviderManager();
    ~ProviderManager();

    void initialize();

    std::shared_ptr<Provider> getCurrentProvider() const;
    std::vector<std::shared_ptr<Provider>> getRegisteredProviders() const;
    void setCurrentProvider(const std::string &providerName);
    std::shared_ptr<Provider> getProvider(const std::string &providerName) const;

    void registerProvider(std::shared_ptr<Provider> provider);
    bool isProviderRegistered(const std::string &providerName) const;
    void unregisterProvider(const std::string &providerName);

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class ProviderManager
} // namespace ai
#endif // PROVIDERMANAGER_H
