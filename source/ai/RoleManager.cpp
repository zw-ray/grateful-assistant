#include <ai/AssistantRole.h>
#include <ai/IntentManager.h>
#include <ai/RoleManager.h>
#include <unordered_map>

#include "roles/AwakeWordVerifyRole.h"
#include "roles/SystemRole.h"

namespace ai {

struct RoleManager::Data
{
    std::unordered_map<std::string, AssistantRole::Ptr> roles;
    std::string currentRoleName;
    IntentManager &intentManager;

    std::shared_ptr<AwakeWordVerifyRole> awakeWordVerifyRole{nullptr};
    std::shared_ptr<SystemRole> systemRole{nullptr};

    Data(IntentManager &intentManager) : intentManager(intentManager) {}
};

RoleManager::RoleManager(IntentManager &intentManager)
    : m_data(std::make_unique<Data>(intentManager))
{
    m_data->awakeWordVerifyRole = std::make_shared<AwakeWordVerifyRole>();
    m_data->systemRole = std::make_shared<SystemRole>(*this, m_data->intentManager);
    registerRole(m_data->awakeWordVerifyRole);
    registerRole(m_data->systemRole);
}

RoleManager::~RoleManager() {}

void RoleManager::initialize()
{
}



AssistantRole::Ptr RoleManager::getCurrentRole() const
{
    if (m_data->currentRoleName.empty())
    {
        return m_data->systemRole;
    }
    else
    {
        return m_data->roles[m_data->currentRoleName];
    }
}

void RoleManager::setCurrentRole(const std::string &roleName)
{
    if (m_data->roles.find(roleName) != m_data->roles.end())
    {
        m_data->currentRoleName = roleName;
    }
    else
    {
        m_data->currentRoleName.clear();
    }
}

AssistantRole::Ptr RoleManager::getRole(const std::string &roleName) const
{
    if (m_data->roles.find(roleName) != m_data->roles.end())
    {
        return m_data->roles[roleName];
    }
    else
    {
        return nullptr;
    }
}

void RoleManager::registerRole(AssistantRole::Ptr role)
{
    m_data->roles[role->getRoleName()] = role;
}

std::vector<AssistantRole::Ptr> RoleManager::getRegisteredRoles() const
{
    std::vector<AssistantRole::Ptr> roles;
    for (const auto &pair : m_data->roles)
    {
        roles.push_back(pair.second);
    }
    return roles;
}

bool RoleManager::isRoleRegistered(const std::string &roleName) const
{
    return m_data->roles.find(roleName) != m_data->roles.end();
}

void RoleManager::unregisterRole(const std::string &roleName)
{
    if (isRoleRegistered(roleName))
    {
        m_data->roles.erase(roleName);
    }
}

} // namespace ai
