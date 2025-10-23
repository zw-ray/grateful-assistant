/*******************************************************************************
**     FileName: AI.h
**    ClassName: AI
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 19:32
**  Description:
*******************************************************************************/

#ifndef AI_H
#define AI_H

#include <ai/AIExport.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ai {

class RoleManager;
class ProviderManager;
class IntentManager;

class AI_API AI
{
    AI();
    ~AI();

public:
    using Ptr = std::shared_ptr<AI>;
    static AI &getInstance();

    bool initialize();

    std::shared_ptr<RoleManager> getRoleManager() const;
    std::shared_ptr<ProviderManager> getProviderManager() const;
    std::shared_ptr<IntentManager> getIntentManager() const;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class AIService

} // namespace ai

#endif // AISERVICE_H
