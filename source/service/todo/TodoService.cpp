#include "TodoAssistant.h"
#include "TodoIntent.h"
#include <ai/AI.h>
#include <ai/IntentManager.h>
#include <ai/RoleManager.h>
#include <algorithm>
#include <chrono>
#include <db/DatabaseConnection.h>
#include <db/DatabaseManager.h>
#include <functional>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/IService.h>
#include <kernel/Logger.h>
#include <kernel/entities/Entity.h>
#include <kernel/entities/Todo.h>
#include <memory>
#include <service/todo/TodoService.h>
#include <vector>

struct TodoService::Data
{
    bool initialized;
    bool running;
    std::shared_ptr<TodoAssistant> todoRole;
    std::shared_ptr<TodoIntent> todoIntent;

    Subscription createTodoSubscription;
    Subscription deleteTodoSubscription;
    Subscription updateTodoSubscription;
    Subscription getTodoSubscription;
    Subscription getAllTodosSubscription;
    Subscription getTodosBySubscription;
    Subscription orderTodosBySubscription;

    void createTodo(const TodoEvents::CreateTodoEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        if (conn->insertIntoTable("todos", event.item))
        {
            TodoEvents::TodoCreatedEvent e;
            e.time = event.time;
            e.item = event.item;
            EventBus::getInstance().publish_async(e);
            Logger::logDebug("TodoService::createTodo: created todo: %s", event.item.title.c_str());
        }
        else
        {
            Logger::logError("TodoService::createTodo: failed to create todo: %s",
                             event.item.title.c_str());
        }
    }
    void deleteTodo(const TodoEvents::DeleteTodoEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct DeleteCondition : public DatabaseConnection::Condition
        {
            std::string column;
            std::string value;

            std::string operator()() const override { return column + " = '" + value + "'"; }
        };
        DeleteCondition dc;
        dc.column = "title";
        dc.value = event.title;
        if (conn->remove("todos", dc))
        {
            TodoEvents::TodoDeletedEvent e;
            e.time = event.time;
            e.title = event.title;
            EventBus::getInstance().publish_async(e);
            Logger::logDebug("TodoService::deleteTodo: deleted todo: %s", event.title.c_str());
        }
        else
        {
            Logger::logError("TodoService::deleteTodo: failed to delete todo: %s",
                             event.title.c_str());
        }
    }
    void updateTodo(const TodoEvents::UpdateTodoEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct UpdateCondition : public DatabaseConnection::Condition
        {
            std::string column;
            Todo item;

            std::string operator()() const override { return column + " = '" + item.title + "'"; }
        };
        UpdateCondition uc;
        uc.column = "title";
        uc.item = event.item;
        if (conn->update("todos", event.item, uc))
        {
            TodoEvents::TodoUpdatedEvent e;
            e.time = event.time;
            e.item = event.item;
            EventBus::getInstance().publish_async(e);
            Logger::logDebug("TodoService::updateTodo: updated todo: %s", event.item.title.c_str());
        }
        else
        {
            Logger::logError("TodoService::updateTodo: failed to update todo: %s",
                             event.item.title.c_str());
        }
    }
    void getTodo(const TodoEvents::GetTodoEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct GetCondition : public DatabaseConnection::Condition
        {
            std::string column;
            std::string value;

            std::string operator()() const override { return column + " = '" + value + "'"; }
        };
        GetCondition gc;
        gc.column = "title";
        gc.value = event.title;
        std::vector<std::shared_ptr<Entity>> results;
        conn->find("todos", gc, results);
        if (results.empty())
        {
            return;
        }
        auto todo = std::dynamic_pointer_cast<Todo>(results[0]);
        if (todo)
        {
            event.receiver(*todo);
        }
    }
    void getAllTodos(const TodoEvents::GetAllTodosEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct FindAllCondition : public DatabaseConnection::Condition
        {
            std::string operator()() const override { return ""; }
        } fac;
        std::vector<std::shared_ptr<Entity>> results;
        conn->find("todos", fac, results);
        if (results.empty())
        {
            return;
        }
        std::vector<Todo> todos;
        for (auto &result : results)
        {
            auto todo = std::dynamic_pointer_cast<Todo>(result);
            if (todo)
            {
                todos.push_back(*todo);
            }
        }
        event.receiver(todos);
    }
    void getTodosBy(const TodoEvents::GetTodosByEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct FindAllCondition : public DatabaseConnection::Condition
        {
            std::string operator()() const override { return ""; }
        } fac;
        std::vector<std::shared_ptr<Entity>> results;
        conn->find("todos", fac, results);
        if (results.empty())
        {
            return;
        }
        std::vector<Todo> todos;
        for (auto &result : results)
        {
            auto todo = std::dynamic_pointer_cast<Todo>(result);
            if (todo)
            {
                todos.push_back(*todo);
            }
        }
        todos.erase(std::remove_if(todos.begin(), todos.end(),
                                   [&](const Todo &todo) { return !event.filter(todo); }),
                    todos.end());
        event.receiver(todos);
    }
    void orderTodosBy(const TodoEvents::OrderTodosByEvent &event)
    {
        auto &db = DatabaseManager::getInstance();
        auto conn = db.getTodoConnection();
        if (!conn)
        {
            return;
        }
        struct FindAllCondition : public DatabaseConnection::Condition
        {
            std::string operator()() const override { return ""; }
        } fac;
        std::vector<std::shared_ptr<Entity>> results;
        conn->find("todos", fac, results);
        if (results.empty())
        {
            return;
        }
        std::vector<Todo> todos;
        for (auto &result : results)
        {
            auto todo = std::dynamic_pointer_cast<Todo>(result);
            if (todo)
            {
                todos.push_back(*todo);
            }
        }
        std::sort(todos.begin(), todos.end(), event.compare);
        event.receiver(todos);
    }

    Data()
        : initialized(false), running(false), createTodoSubscription([]() {}),
          deleteTodoSubscription([]() {}), updateTodoSubscription([]() {}),
          getTodoSubscription([]() {}), getAllTodosSubscription([]() {}),
          getTodosBySubscription([]() {}), orderTodosBySubscription([]() {}), todoRole(nullptr)
    {
        createTodoSubscription = EventBus::getInstance().on<TodoEvents::CreateTodoEvent>(
            std::bind(&TodoService::Data::createTodo, this, std::placeholders::_1));
        deleteTodoSubscription = EventBus::getInstance().on<TodoEvents::DeleteTodoEvent>(
            std::bind(&TodoService::Data::deleteTodo, this, std::placeholders::_1));
        updateTodoSubscription = EventBus::getInstance().on<TodoEvents::UpdateTodoEvent>(
            std::bind(&TodoService::Data::updateTodo, this, std::placeholders::_1));
        getTodoSubscription = EventBus::getInstance().on<TodoEvents::GetTodoEvent>(
            std::bind(&TodoService::Data::getTodo, this, std::placeholders::_1));
        getAllTodosSubscription = EventBus::getInstance().on<TodoEvents::GetAllTodosEvent>(
            std::bind(&TodoService::Data::getAllTodos, this, std::placeholders::_1));
        getTodosBySubscription = EventBus::getInstance().on<TodoEvents::GetTodosByEvent>(
            std::bind(&TodoService::Data::getTodosBy, this, std::placeholders::_1));
        orderTodosBySubscription = EventBus::getInstance().on<TodoEvents::OrderTodosByEvent>(
            std::bind(&TodoService::Data::orderTodosBy, this, std::placeholders::_1));

#ifdef GA_DEBUG
        TodoEvents::CreateTodoEvent e;
        e.item.title = u8"开会";
        e.item.content = u8"这是一个非常长的开会内容的描述，用于测试是否能够正常显示。你还好吗？我"
                         u8"很好谢谢。你真的还好吗？我假的很好。";
        auto now = std::chrono::system_clock::now();
        auto dueTime =
            std::chrono::time_point_cast<std::chrono::seconds>(now + std::chrono::hours(8));
        e.item.dueTime = dueTime;
        e.item.reminderTime =
            std::chrono::time_point_cast<std::chrono::seconds>(dueTime - std::chrono::hours(1));
        e.item.priority = Todo::Priority::kImportantAndUrgent;
        e.item.status = Todo::Status::kNotStarted;
        e.item.createdAt = std::chrono::time_point_cast<std::chrono::seconds>(now);
        e.item.updatedAt = e.item.createdAt;
        createTodo(e);
#endif
    }
};

TodoService::TodoService() : m_data(std::make_unique<Data>())
{
    m_data->initialized = false;
    m_data->running = false;
}

TodoService::~TodoService() { shutdown(); }

bool TodoService::initialize()
{
    // 1. 向AI服务注册意图
    auto intentManager = ai::AI::getInstance().getIntentManager();
    m_data->todoIntent = std::make_shared<TodoIntent>();
    intentManager->registerIntent(m_data->todoIntent);

    // 2. 初始化助手角色，然后向AI模块注册角色
    m_data->todoRole = std::make_shared<TodoAssistant>();
    auto roleManager = ai::AI::getInstance().getRoleManager();
    roleManager->registerRole(m_data->todoRole);

    m_data->initialized = true;
    return m_data->initialized;
}

bool TodoService::start()
{
    if (!m_data->initialized)
    {
        return false;
    }
    m_data->running = true;
    return true;
}

void TodoService::shutdown()
{
    if (m_data)
    {
        m_data->createTodoSubscription.unsubscribe();
        m_data->deleteTodoSubscription.unsubscribe();
        m_data->updateTodoSubscription.unsubscribe();
        m_data->getTodoSubscription.unsubscribe();
        m_data->getAllTodosSubscription.unsubscribe();
        m_data->getTodosBySubscription.unsubscribe();
        m_data->orderTodosBySubscription.unsubscribe();

        // 3. 从AI服务注销意图
        auto intentManager = ai::AI::getInstance().getIntentManager();
        intentManager->unregisterIntent(m_data->todoIntent->getName());
        // 4. 从AI服务注销角色
        auto roleManager = ai::AI::getInstance().getRoleManager();
        roleManager->unregisterRole(m_data->todoRole->getRoleName());
        m_data->initialized = false;
    }
    if (m_data->running)
    {
        m_data->running = false;
    }
}
bool TodoService::isInitialized() const { return m_data->initialized; }

bool TodoService::isRunning() const { return m_data->running; }

IService *createService() { return new TodoService; }

void destroyService(IService *service) { delete service; }
