/*******************************************************************************
**     FileName: RoleManager.h
**    ClassName: RoleManager
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/14 18:30
**  Description:
*******************************************************************************/

#ifndef ROLEMANAGER_H
#define ROLEMANAGER_H

#include <ai/AIExport.h>
#include <memory>
#include <string>
#include <vector>

#include <ai/AssistantRole.h>
#include <ai/IntentManager.h>

namespace ai {

class AI_API RoleManager
{
public:
    RoleManager(IntentManager &intentManager);
    ~RoleManager();

    void initialize();

    AssistantRole::Ptr getCurrentRole() const;
    void setCurrentRole(const std::string &roleName);
    AssistantRole::Ptr getRole(const std::string &roleName) const;

    void registerRole(AssistantRole::Ptr role);
    std::vector<AssistantRole::Ptr> getRegisteredRoles() const;
    bool isRoleRegistered(const std::string &roleName) const;
    void unregisterRole(const std::string &roleName);

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class RoleManager

} // namespace ai

#endif // ROLEMANAGER_H
