/*******************************************************************************
**     FileName: TodoService.h
**    ClassName: TodoService
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:41
**  Description:
*******************************************************************************/

#ifndef TODOSERVICE_H
#define TODOSERVICE_H

#include <kernel/IService.h>
#include <memory>

class TodoService : public IService
{
public:
    TodoService();
    ~TodoService();

    std::string getName() const override { return "todo"; }
    bool initialize() override;
    bool start() override;
    void shutdown() override;
    bool isInitialized() const override;
    bool isRunning() const override;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class TodoService

SERVICE_EXPORT_FUNCTION()

#endif // TODOSERVICE_H
