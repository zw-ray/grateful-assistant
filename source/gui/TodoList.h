/*******************************************************************************
**     FileName: TodoList.h
**    ClassName: TodoList
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:37
**  Description: 待办项列表组件
*******************************************************************************/
#ifndef TODOLIST_H
#define TODOLIST_H

#include "Card.h"
#include <QListWidget>
#include <QWidget>
#include <kernel/Events.h>

class TodoList : public Card
{
    Q_OBJECT
public:
    TodoList(QWidget *parent = nullptr);
    ~TodoList() override;

protected:
    void initUI();

signals:
    void todoCreated(const Todo &todo);
    void todoDeleted(const std::string &title);
    void todoUpdated(const Todo &todo);

public slots:
    void addTodoItem(const Todo &todo);

private slots:
    // 处理完成按钮点击
    void onCompleteClicked(const Todo &todo);
    void slotOnTodoCreated(const Todo &todo);
    void slotOnTodoDeleted(const std::string &title);
    void slotOnTodoUpdated(const Todo &todo);

protected:
    void onTodoCreated(const TodoEvents::TodoCreatedEvent &event);
    void onTodoDeleted(const TodoEvents::TodoDeletedEvent &event);
    void onTodoUpdated(const TodoEvents::TodoUpdatedEvent &event);

private:
    struct Data;
    std::unique_ptr<Data> m_data;
};

#endif // TODOLIST_H
