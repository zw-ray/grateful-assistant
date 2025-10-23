/*******************************************************************************
**     FileName: Todo.h
**    ClassName: Todo
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 13:16
**  Description:
*******************************************************************************/

#ifndef TODO_H
#define TODO_H

#include <chrono>
#include <cstdint>
#include <kernel/entities/Entity.h>
#include <string>

struct Todo : public Entity
{
    using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;
    int32_t id;             // 主键，自增ID
    std::string title;      // 标题
    std::string content;    // 内容
    TimePoint dueTime;      // 截止时间
    TimePoint reminderTime; // 提醒时间

    enum class Priority
    {
        kImportantAndUrgent = 0,
        kImportantButNotUrgent = 1,
        kNotImportantButUrgent = 2,
        kNotImportantAndNotUrgent = 3,
    };

    Priority priority; // 优先级

    enum class Status
    {
        kNotStarted = 0,
        kInProgress,
        kCompleted,
        kOverdue = 3,
    };
    Status status;       // 状态
    TimePoint createdAt; // 创建时间
    TimePoint updatedAt; // 更新时间

    operator std::string() const { return "todos"; }
};

#endif // TODO_H
