/*******************************************************************************
**     FileName: DatabaseManager.h
**    ClassName: DatabaseManager
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 15:28
**  Description:
*******************************************************************************/

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <db/DatabaseConnection.h>
#include <db/DatabaseExport.h>
#include <memory>

class DATABASE_API DatabaseManager
{
    DatabaseManager();
    ~DatabaseManager();

public:
    static DatabaseManager &getInstance();

    class ConnectionRegistry
    {
    public:
        virtual ~ConnectionRegistry() = default;

        using Ptr = std::shared_ptr<ConnectionRegistry>;
        virtual std::shared_ptr<DatabaseConnection>
            createConnection() const = 0;
        virtual std::string getName() const = 0;
    };

    DatabaseConnection::Ptr getTodoConnection() const;
    DatabaseConnection::Ptr
        getModelInfoConnection(const std::string &provider) const;

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class DatabaseManager

#endif // DATABASEMANAGER_H
