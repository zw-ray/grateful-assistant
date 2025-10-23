#include <db/DatabaseManager.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SiliconFlowModelInfoConnection.h"
#include "TodoConnection.h"
#include "db/DatabaseConnection.h"

struct DatabaseManager::Data
{
    std::map<std::string, ConnectionRegistry::Ptr> registries;
};

class TodoRegistry : public DatabaseManager::ConnectionRegistry
{
public:
    std::shared_ptr<DatabaseConnection> createConnection() const override
    {
        return std::make_shared<TodoConnection>();
    }

    std::string getName() const override { return "todo"; }
};

class ModelInfoRegistry : public DatabaseManager::ConnectionRegistry
{
public:
    std::shared_ptr<DatabaseConnection> createConnection() const override
    {
        if (m_providerName.empty())
        {
            return std::make_shared<SiliconFlowModelInfoConnection>();
        }
        else
        {
            if (isProviderAccepted(m_providerName))
            {
                if (m_providerName == "siliconflow")
                {
                    return std::make_shared<SiliconFlowModelInfoConnection>();
                }
            }
        }
        return nullptr;
    }

    std::string getName() const override { return "model_info"; }

    void setProviderName(std::string providerName)
    {
        m_providerName = providerName;
    }

    static bool isProviderAccepted(const std::string &providerName)
    {
        const std::vector<std::string> acceptedProviders = {
            "siliconflow",
        };
        return std::find(acceptedProviders.begin(), acceptedProviders.end(),
                         providerName) != acceptedProviders.end();
    }

protected:
    std::string m_providerName;
};

DatabaseManager::DatabaseManager() : m_data(std::make_unique<Data>())
{
    auto reg = std::make_shared<TodoRegistry>();
    m_data->registries[reg->getName()] = reg;
    auto reg2 = std::make_shared<ModelInfoRegistry>();
    m_data->registries[reg2->getName()] = reg2;
}

DatabaseManager::~DatabaseManager() {}

DatabaseManager &DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseConnection::Ptr DatabaseManager::getTodoConnection() const
{
    return m_data->registries["todo"]->createConnection();
}

DatabaseConnection::Ptr
    DatabaseManager::getModelInfoConnection(const std::string &provider) const
{
    auto miReg = std::dynamic_pointer_cast<ModelInfoRegistry>(
        m_data->registries["model_info"]);
    miReg->setProviderName(provider);
    return miReg->createConnection();
}
