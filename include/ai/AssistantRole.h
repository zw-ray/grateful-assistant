/*******************************************************************************
**     FileName: AssistantRole.h
**    ClassName: AssistantRole
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 21:34
**  Description:
*******************************************************************************/

#ifndef ASSISTANTROLE_H
#define ASSISTANTROLE_H

#include <ai/AIExport.h>
#include <ai/Model.h>
#include <memory>
#include <string>

namespace ai {

class Intent;

class AI_API AssistantRole
{
public:
    using Ptr = std::shared_ptr<AssistantRole>;
    virtual ~AssistantRole() = default;

    virtual std::string getRoleName() const { return m_roleName; }
    virtual std::string getAssistantRoleDescription() const { return m_roleDescription; }
    virtual std::string handleRequest(Model::Ptr model, const std::string &request) = 0;
    virtual bool acceptIntent(std::shared_ptr<Intent> intent) const = 0;

protected:
    std::string m_roleName;        // 角色名称
    std::string m_roleDescription; // 角色描述
}; // class AssistantRole

} // namespace ai
#endif // ASSISTANTROLE_H
