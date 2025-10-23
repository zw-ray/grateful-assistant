/*******************************************************************************
**     FileName: TodoIntent.h
**    ClassName: TodoIntent
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/15 14:21
**  Description:
*******************************************************************************/

#ifndef TODOINTENT_H
#define TODOINTENT_H

#include <ai/Intent.h>

class TodoIntent : public ai::Intent
{
public:
    TodoIntent() {}
    ~TodoIntent() {}

    std::string getName() const override { return "todo"; }
    std::string getDescription() const override { return u8"处理待办事项（TODO）相关的任务"; }

}; // class TodoIntent

#endif // TODOINTENT_H
