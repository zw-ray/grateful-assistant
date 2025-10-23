/*******************************************************************************
**     FileName: SiliconFlowModelInfoConnection.h
**    ClassName: SiliconFlowModelInfoConnection
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 15:38
**  Description:
*******************************************************************************/

#ifndef SILICONFLOWMODELINFOCONNECTION_H
#define SILICONFLOWMODELINFOCONNECTION_H

#include <db/DatabaseConnection.h>

class SiliconFlowModelInfoConnection : public DatabaseConnection
{
public:
    SiliconFlowModelInfoConnection();
    ~SiliconFlowModelInfoConnection();

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
}; // class SiliconFlowModelInfoConnection

#endif // SILICONFLOWMODELINFOCONNECTION_H
