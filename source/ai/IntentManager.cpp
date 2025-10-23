#include <ai/Intent.h>
#include <ai/IntentManager.h>
#include <string>
#include <unordered_map>

namespace ai {

struct IntentManager::Data
{
    std::unordered_map<std::string, Intent::Ptr> intents;
};

IntentManager::IntentManager() : m_data(std::make_unique<Data>()) {}

IntentManager::~IntentManager() {}

void IntentManager::initialize()
{
}

void IntentManager::registerIntent(std::shared_ptr<Intent> intent)
{
    m_data->intents[intent->getName()] = intent;
}

std::shared_ptr<Intent> IntentManager::getIntent(const std::string &intentName) const
{
    auto iter = m_data->intents.find(intentName);
    if (iter == m_data->intents.end())
    {
        return nullptr;
    }
    return iter->second;
}

std::vector<std::shared_ptr<Intent>> IntentManager::getIntents() const
{
    std::vector<std::shared_ptr<Intent>> ret;
    for (auto &iter : m_data->intents)
    {
        ret.push_back(iter.second);
    }
    return ret;
}

bool IntentManager::hasIntent(const std::string &intentName) const
{
    return m_data->intents.find(intentName) != m_data->intents.end();
}

void IntentManager::unregisterIntent(const std::string &intentName)
{
    m_data->intents.erase(intentName);
}

} // namespace ai
