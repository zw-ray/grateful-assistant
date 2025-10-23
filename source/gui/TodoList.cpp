#include "TodoList.h"
#include "TodoListItem.h"
#include "kernel/EventBus.h"
#include "kernel/Events.h"

#include <QHBoxLayout>
#include <QListWidget>

#include <functional>

struct TodoList::Data
{
    QLabel *title;
    QListWidget *listWidget;

    Subscription todoCreatedSubscription;
    Subscription todoDeletedSubscription;
    Subscription todoUpdatedSubscription;

    Data()
        : listWidget(nullptr), todoCreatedSubscription([]() {}), todoDeletedSubscription([]() {}),
          todoUpdatedSubscription([]() {})
    {
    }
};

TodoList::TodoList(QWidget *parent) : Card(parent), m_data(std::make_unique<Data>()) { initUI(); }

TodoList::~TodoList() = default;

void TodoList::initUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_data->title = new QLabel("待办事项");
    m_data->title->setStyleSheet("font-size: 16pt; "
                                 "font-weight: bold; "
                                 "padding: 10px;");
    layout->addWidget(m_data->title);

    m_data->listWidget = new QListWidget();
    m_data->listWidget->setStyleSheet("background-color: transparent; "
                                      "border: none; "
                                      "padding: 10px;");
    m_data->listWidget->setSpacing(10);
    layout->addWidget(m_data->listWidget);
    setLayout(layout);

    m_data->todoCreatedSubscription = EventBus::getInstance().on<TodoEvents::TodoCreatedEvent>(
        std::bind(&TodoList::onTodoCreated, this, std::placeholders::_1));
    m_data->todoDeletedSubscription = EventBus::getInstance().on<TodoEvents::TodoDeletedEvent>(
        std::bind(&TodoList::onTodoDeleted, this, std::placeholders::_1));
    m_data->todoUpdatedSubscription = EventBus::getInstance().on<TodoEvents::TodoUpdatedEvent>(
        std::bind(&TodoList::onTodoUpdated, this, std::placeholders::_1));

    connect(this, SIGNAL(todoCreated(const Todo &)), this, SLOT(slotOnTodoCreated(const Todo &)));
    connect(this, SIGNAL(todoDeleted(const std::string &)), this,
            SLOT(slotOnTodoDeleted(const std::string &)));
    connect(this, SIGNAL(todoUpdated(const Todo &)), this, SLOT(slotOnTodoUpdated(const Todo &)));

    TodoEvents::GetTodosByEvent getTodosEvent;
    getTodosEvent.filter = [](const Todo &todo)
    {
        return todo.status == Todo::Status::kInProgress ||
               todo.status == Todo::Status::kNotStarted || todo.status == Todo::Status::kOverdue;
    };
    getTodosEvent.sql = "SELECT * FROM todos WHERE status IN (0, 1, 3)";
    getTodosEvent.receiver = [this](const std::vector<Todo> &todos)
    {
        for (const auto &todo : todos)
        {
            addTodoItem(todo);
        }
    };
    EventBus::getInstance().publish<TodoEvents::GetTodosByEvent>(getTodosEvent);
}

void TodoList::addTodoItem(const Todo &todo)
{
    // 创建列表项容器
    auto *listItem = new QListWidgetItem(m_data->listWidget);
    // 创建自定义部件
    auto *todoItem = new TodoListItem(todo);
    listItem->setSizeHint(todoItem->sizeHint());
    // 将自定义部件设置到列表项中
    m_data->listWidget->setItemWidget(listItem, todoItem);
}

void TodoList::onCompleteClicked(const Todo &todo)
{
    // 处理完成逻辑（例如更新进度为100%）
    Todo updatedTodo = todo;
    updatedTodo.status = Todo::Status::kCompleted;

    // 可以发送事件通知内核更新数
    TodoEvents::UpdateTodoEvent updateEvent;
    updateEvent.item = updatedTodo;
    updateEvent.title = updatedTodo.title;
    updateEvent.time = std::time(nullptr);
    EventBus::getInstance().publish_async(updateEvent);
}

void TodoList::slotOnTodoCreated(const Todo &todo) { addTodoItem(todo); }

void TodoList::slotOnTodoDeleted(const std::string &title)
{
    auto item = m_data->listWidget->findItems(QString::fromUtf8(title), Qt::MatchExactly).first();
    if (item)
    {
        // 从列表中移除项
        auto *itemWidget = m_data->listWidget->takeItem(m_data->listWidget->row(item));
        delete itemWidget;
    }
}

void TodoList::slotOnTodoUpdated(const Todo &todo)
{
    auto item =
        m_data->listWidget->findItems(QString::fromUtf8(todo.title), Qt::MatchExactly).first();
    if (item)
    {
        // 更新列表项
        auto *todoItem = qobject_cast<TodoListItem *>(m_data->listWidget->itemWidget(item));
        if (todoItem)
        {
            todoItem->updateItem(todo);
        }
    }
}

// 其他事件处理方法（创建/删除/更新）
void TodoList::onTodoCreated(const TodoEvents::TodoCreatedEvent &event)
{
    emit todoCreated(event.item);
}

void TodoList::onTodoUpdated(const TodoEvents::TodoUpdatedEvent &event)
{
    emit todoUpdated(event.item);
}

void TodoList::onTodoDeleted(const TodoEvents::TodoDeletedEvent &event)
{
    emit todoDeleted(event.title);
}
