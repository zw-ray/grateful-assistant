/*******************************************************************************
**     FileName: TodoConnection.h
**    ClassName: TodoConnection
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 15:36
**  Description:
*******************************************************************************/

#ifndef TODOCONNECTION_H
#define TODOCONNECTION_H

#include <db/DatabaseConnection.h>
#include <memory>

class TodoConnection : public DatabaseConnection
{
public:
    TodoConnection();
    ~TodoConnection();

    std::vector<std::string> getTableNames() const override;
    bool insertIntoTable(const std::string &tableName,
                         const Entity &entity) override;
    bool getAll(const std::string &tableName,
                std::vector<std::shared_ptr<Entity>> &entities) override;

    bool find(const std::string &tableName, const Condition &condition,
              std::vector<std::shared_ptr<Entity>> &entities) override;
    bool update(const std::string &tableName, const Entity &entity,
                const Condition &condition) override;
    bool remove(const std::string &tableName,
                const Condition &condition) override;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class TodoConnection

#endif // TODOCONNECTION_H
