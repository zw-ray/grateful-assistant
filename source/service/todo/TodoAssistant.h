/*******************************************************************************
**     FileName: TodoAssistant.h
**    ClassName: TodoAssistant
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 21:35
**  Description:
*******************************************************************************/

#ifndef TODOASSISTANT_H
#define TODOASSISTANT_H

#include <ai/AssistantRole.h>
#include <ai/Intent.h>
#include <kernel/entities/Todo.h>
#include <map>

/**
 * @brief
 * 专门处理待办项相关任务的助手，比如新建待办项、查看待办项、删除待办项等。
 * 这是系统角色经过意图识别之后，进行调用的角色。
 *
 * @details
 * 例如：用户通过语音输入，识别出来的文本是：为我创建一个待办项，明天上午九点开会
 * 经过系统识别后，意图为：待办项
 * Todo助手将会根据“为我创建一个待办项，明天上午九点开会”这段话，让大模型将其解析为
 * 具体的待办项信息，并将其存储到待办项列表中。
 * 需要的大模型返回的格式为（json）：
 * {
 *     "title": "开会",
 *     "description": "明天上午九点开会",
 *     "due_date": "2025-10-04 09:00:00",
 *     "priority": "高",
 *     "status": "未完成",
 *     "operation": "add"
 * }
 */
class TodoAssistant : public ai::AssistantRole
{
public:
    TodoAssistant();
    ~TodoAssistant();

    std::string getRoleName() const override;
    std::string getAssistantRoleDescription() const override;
    std::string handleRequest(ai::Model::Ptr model, const std::string &request) override;
    bool acceptIntent(ai::Intent::Ptr intent) const override;

    class TodoOperator
    {
    public:
        using Ptr = std::shared_ptr<TodoOperator>;
        virtual ~TodoOperator() = default;
        virtual void addParam(const std::string &key, const std::string &value)
        {
            m_params[key] = value;
        }
        virtual bool parseParams(const std::string &response) = 0;
        virtual bool execute() = 0;

    protected:
        std::map<std::string, std::string> m_params;
    };

protected:
    void handleResponse(const std::string &response);

private:
    std::map<std::string, TodoOperator::Ptr> m_operationMap;
}; // class TodoAssistant
#endif // TODOASSISTANT_H
