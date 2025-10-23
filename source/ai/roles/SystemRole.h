/*******************************************************************************
**     FileName: SystemRole.h
**    ClassName: SystemRole
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 21:39
**  Description:
*******************************************************************************/

#ifndef SYSTEMROLE_H
#define SYSTEMROLE_H

#include "ai/IntentManager.h"
#include "ai/RoleManager.h"
#include <ai/AssistantRole.h>
#include <ai/Intent.h>
#include <memory>

namespace ai {

/**
 * @brief 处理音频识别文本，转换成意图，并分解为具体的任务或指令。
 * 例如用户通过语音输入“明天早上九点开会”，系统助手会向大模型发送
 * 一个意图解析的请求：“将这段话分解为具体意图（system/todo）：” +
 * "明天早上九点开会"
 * 然后大模型会返回结果：
 * {
 *     "intent": "todo"
 * },
 * 接下来系统助手会调用Todo助手的处理请求的方法：TodoAssistant::handleRequest("明天早上九点开会");
 * 剩下的事情就是TodoAssistant要做的了。
 */
class SystemRole : public AssistantRole
{
public:
    SystemRole(RoleManager &roleManager, IntentManager &intentManager);
    ~SystemRole();

    std::string getRoleName() const override;
    virtual std::string getAssistantRoleDescription() const override;
    virtual std::string handleRequest(Model::Ptr model, const std::string &request) override;
    bool acceptIntent(Intent::Ptr intent) const override;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class SystemRole
} // namespace ai
#endif // SYSTEMROLE_H
