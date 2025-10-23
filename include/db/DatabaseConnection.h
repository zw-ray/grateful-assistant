/*******************************************************************************
**     FileName: DatabaseConnection.h
**    ClassName: DatabaseConnection
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 15:28
**  Description:
*******************************************************************************/

#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <db/DatabaseExport.h>
#include <functional>
#include <kernel/entities/Entity.h>
#include <memory>
#include <string>
#include <vector>

class DATABASE_API DatabaseConnection
{
public:
    using Ptr = std::shared_ptr<DatabaseConnection>;
    DatabaseConnection() = default;
    virtual ~DatabaseConnection() = default;

    virtual std::vector<std::string> getTableNames() const = 0;
    virtual bool insertIntoTable(const std::string &tableName,
                                 const Entity &entity) = 0;
    virtual bool getAll(const std::string &tableName,
                        std::vector<std::shared_ptr<Entity>> &entities) = 0;

    struct Condition
    {
        virtual ~Condition() = default;
        virtual std::string operator()() const { return ""; }
    };
    virtual bool find(const std::string &tableName, const Condition &condition,
                      std::vector<std::shared_ptr<Entity>> &entities) = 0;
    virtual bool update(const std::string &tableName, const Entity &entity,
                        const Condition &condition) = 0;
    virtual bool remove(const std::string &tableName,
                        const Condition &condition) = 0;

}; // class DatabaseConnection

#endif // DATABASECONNECTION_H
