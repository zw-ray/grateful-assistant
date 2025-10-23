#include "TodoAssistant.h"
#include "ai/Intent.h"
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/entities/Todo.h>

#include <date/date.h>
#include <date/tz.h>

#include <chrono>

#include <fmt/chrono.h>
#include <fstream>
#include <kernel/Logger.h>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace {

std::string formatTime()
{
    auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
    auto now_truncated = std::chrono::floor<std::chrono::seconds>(now);
    const std::string dateTime = fmt::format("{:%Y_%m_%d_%H_%M_%S}", now_truncated);
    return dateTime;
}

Todo::Priority parsePriority(const std::string &priority)
{
    if (priority == "kImportantAndUrgent" || priority == "0")
    {
        return Todo::Priority::kImportantAndUrgent;
    }
    else if (priority == "kImportantButNotUrgent" || priority == "1")
    {
        return Todo::Priority::kImportantButNotUrgent;
    }
    else if (priority == "kNotImportantButUrgent" || priority == "2")
    {
        return Todo::Priority::kNotImportantButUrgent;
    }
    else if (priority == "kNotImportantAndNotUrgent" || priority == "3")
    {
        return Todo::Priority::kNotImportantAndNotUrgent;
    }
    else
    {
        return Todo::Priority::kImportantButNotUrgent;
    }
}

Todo::Status parseStatus(const std::string &status)
{
    if (status == "kNotStarted" || status == "0")
    {
        return Todo::Status::kNotStarted;
    }
    else if (status == "kInProgress" || status == "1")
    {
        return Todo::Status::kInProgress;
    }
    else if (status == "kCompleted" || status == "2")
    {
        return Todo::Status::kCompleted;
    }
    else if (status == "kOverdue" || status == "3")
    {
        return Todo::Status::kOverdue;
    }
    else
    {
        return Todo::Status::kNotStarted;
    }
}

class CreateTodoOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~CreateTodoOperator() = default;
    bool parseParams(const std::string &response) override
    {
        try
        {
            json j = json::parse(response);
            if (j["action"]["name"] != "create")
            {
                Logger::logError("CreateTodoOperator::parseParams: action name not "
                                 "createTodo");
                return false;
            }
            const auto params = j["action"]["params"];
            m_params["title"] = params["title"].get<std::string>();
            m_params["content"] = params["content"].get<std::string>();
            m_params["dueTime"] = params["dueTime"].get<std::string>();
            m_params["reminderTime"] = params["reminderTime"].get<std::string>();
            m_params["priority"] = params["priority"].get<std::string>();
            if (!params.contains("status"))
            {
                m_params["status"] = "0";
            }
            else
            {
                m_params["status"] = params["status"].get<std::string>();
            }
            m_params["createdAt"] = formatTime();
            m_params["updatedAt"] = formatTime();
            return true;
        }
        catch (const json::exception &e)
        {
            Logger::logError("CreateTodoOperator::parseParams: {}", e.what());
            return false;
        }
    }
    virtual bool execute() override
    {
        Todo todo;
        todo.title = m_params["title"];
        todo.content = m_params["content"];
        std::string dueTime = m_params["dueTime"];
        std::istringstream dueTimeStream(dueTime);
        date::sys_seconds tp;
        dueTimeStream >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.dueTime = tp;
        std::string reminderTime = m_params["reminderTime"];
        std::istringstream reminderTimeStream(reminderTime);
        reminderTimeStream >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.reminderTime = tp;
        todo.priority = parsePriority(m_params["priority"]);
        todo.status = parseStatus(m_params["status"]);
        todo.createdAt =
            std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
        todo.updatedAt = todo.createdAt;

        TodoEvents::CreateTodoEvent createEvent;
        createEvent.time = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        createEvent.item = todo;
        EventBus::getInstance().publish_async<TodoEvents::CreateTodoEvent>(createEvent);
        return true;
    }
};
class DeleteTodoOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~DeleteTodoOperator() = default;
    bool parseParams(const std::string &response) override
    {
        try
        {
            json j = json::parse(response);
            if (j["action"]["name"] != "delete")
            {
                Logger::logError("DeleteTodoOperator::parseParams: action name not "
                                 "deleteTodo");
                return false;
            }
            const auto params = j["action"]["params"];
            m_params["title"] = params["title"].get<std::string>();
            return true;
        }
        catch (const json::exception &e)
        {
            Logger::logError("DeleteTodoOperator::parseParams: {}", e.what());
            return false;
        }
    }
    virtual bool execute() override
    {
        TodoEvents::DeleteTodoEvent deleteEvent;
        deleteEvent.time = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        deleteEvent.title = m_params["title"];
        EventBus::getInstance().publish_async<TodoEvents::DeleteTodoEvent>(deleteEvent);
        return true;
    }
};

class UpdateTodoOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~UpdateTodoOperator() = default;
    bool parseParams(const std::string &response) override
    {
        try
        {
            json j = json::parse(response);
            if (j["action"]["name"] != "update")
            {
                Logger::logError("UpdateTodoOperator::parseParams: action name not "
                                 "updateTodo");
                return false;
            }
            const auto params = j["action"]["params"];
            m_params["title"] = params["title"].get<std::string>();
            // 从json的params中遍历所有key-value对
            for (auto it = params.begin(); it != params.end(); ++it)
            {
                m_params[it.key()] = it.value().get<std::string>();
            }
            m_params["updatedAt"] = formatTime();
            return true;
        }
        catch (const json::exception &e)
        {
            Logger::logError("UpdateTodoOperator::parseParams: {}", e.what());
            return false;
        }
    }
    virtual bool execute() override
    {
        Todo todo;
        if (!getTodo(m_params["title"], todo))
        {
            Logger::logError("UpdateTodoOperator::execute: failed to get todo");
            return false;
        }
        if (m_params.find("content") != m_params.end())
        {
            todo.content = m_params["content"];
        }
        if (m_params.find("dueTime") != m_params.end())
        {
            date::sys_seconds tp;
            std::istringstream dueTimeStream(m_params["dueTime"]);
            dueTimeStream >> date::parse("%Y-%m-%d %H:%M:%S", tp);
            todo.dueTime = tp;
        }
        if (m_params.find("reminderTime") != m_params.end())
        {
            date::sys_seconds tp;
            std::istringstream reminderTimeStream(m_params["reminderTime"]);
            reminderTimeStream >> date::parse("%Y-%m-%d %H:%M:%S", tp);
            todo.reminderTime = tp;
        }
        if (m_params.find("priority") != m_params.end())
        {
            todo.priority = parsePriority(m_params["priority"]);
        }
        if (m_params.find("status") != m_params.end())
        {
            todo.status = parseStatus(m_params["status"]);
        }

        TodoEvents::UpdateTodoEvent updateEvent;
        updateEvent.time = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        updateEvent.item = todo;
        EventBus::getInstance().publish_async<TodoEvents::UpdateTodoEvent>(updateEvent);
        return true;
    }

protected:
    bool getTodo(const std::string &title, Todo &todo)
    {
        TodoEvents::GetTodoEvent getEvent;
        getEvent.time = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
        getEvent.title = title;
        getEvent.receiver = [&todo](const Todo &item) { todo = item; };
        EventBus::getInstance().publish_async<TodoEvents::GetTodoEvent>(getEvent);
        return true;
    }
};

class GetByOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~GetByOperator() = default;
    bool parseParams(const std::string &response) override
    {
        try
        {
            json j = json::parse(response);
            if (j["action"]["name"] != "get_by")
            {
                Logger::logError("GetByOperator::parseParams: action name not get_by");
                return false;
            }
            // 从json的params中遍历所有key-value对
            const auto params = j["action"]["params"];
            for (auto it = params.begin(); it != params.end(); ++it)
            {
                m_params[it.key()] = it.value().get<std::string>();
            }
            return true;
        }
        catch (const json::exception &e)
        {
            Logger::logError("GetByOperator::parseParams: {}", e.what());
            return false;
        }
    }
    virtual bool execute() override
    {
        TodoEvents::GetTodosByEvent getEvent;
        getEvent.time = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
        getEvent.sql = m_params["sql"];
        return true;
    }
};

class GetAllOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~GetAllOperator() = default;
    bool parseParams(const std::string &response) override { return true; }
    virtual bool execute() override { return true; }
};

class OrderByOperator : public TodoAssistant::TodoOperator
{
public:
    virtual ~OrderByOperator() = default;
    bool parseParams(const std::string &response) override { return true; }
    virtual bool execute() override { return true; }
};
} // namespace

TodoAssistant::TodoAssistant() : AssistantRole()
{
    m_roleName = "TodoAssistant";
    m_roleDescription = "Todo assistant that handles todo-related tasks";
    m_operationMap["create"] = std::make_shared<CreateTodoOperator>();
    m_operationMap["delete"] = std::make_shared<DeleteTodoOperator>();
    m_operationMap["update"] = std::make_shared<UpdateTodoOperator>();
    m_operationMap["get_by"] = std::make_shared<GetByOperator>();
    m_operationMap["get_all"] = std::make_shared<GetAllOperator>();
    m_operationMap["order_by"] = std::make_shared<OrderByOperator>();
}

TodoAssistant::~TodoAssistant() {}

std::string TodoAssistant::getRoleName() const { return m_roleName; }

std::string TodoAssistant::getAssistantRoleDescription() const { return m_roleDescription; }

std::string TodoAssistant::handleRequest(ai::Model::Ptr model, const std::string &request)
{
    std::string prompt = "";
    {
        // 解析待办项操作 - 从软件运行目录下的prompts目录下读取todo.prompt文件，
        // 然后将request替换到prompt中的{{user_input}}位置
        std::ifstream file("prompts/todo.prompt");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                prompt += line + "\n";
            }
            file.close();
        }
        else
        {
            Logger::logError("AwakeWordVerifyRole::handleRequest: wake_word.prompt file "
                             "not found");
            return "";
        }
        // 字符串替换
        while (prompt.find("{{user_input}}") != std::string::npos)
        {
            prompt.replace(prompt.find("{{user_input}}"), std::string("{{user_input}}").length(),
                           request);
        }
    }
    ai::Model::ModelGenerateResult result = model->text2Text(prompt);
    if (result.isSuccess())
    {
        Logger::logDebug("TodoAssistant::handleRequest: text2Text success, output: {}",
                         result.response);
        handleResponse(result.response);
        return result.response;
    }
    else
    {
        Logger::logError("TodoAssistant::handleRequest: text2Text failed, error: {}", result.error);
        return "";
    }
}

bool TodoAssistant::acceptIntent(ai::Intent::Ptr intent) const
{
    return intent->getName() == "todo";
}

void TodoAssistant::handleResponse(const std::string &response)
{
    using json = nlohmann::json;
    try
    {
        json j = json::parse(response);
        std::string actionName = j["action"]["name"].get<std::string>();
        auto it = m_operationMap.find(actionName);
        if (it != m_operationMap.end())
        {
            if (it->second->parseParams(response))
            {
                it->second->execute();
            }
        }
        else
        {
            Logger::logError("TodoAssistant::handleResponse: unknown action name: {}", actionName);
        }
    }
    catch (json::parse_error &e)
    {
        Logger::logError("TodoAssistant::handleResponse: parse json failed, error: {}", e.what());
        return;
    }
}
