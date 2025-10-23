/*******************************************************************************
**     FileName: IntentManager.h
**    ClassName: IntentManager
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/14 18:47
**  Description:
*******************************************************************************/

#ifndef INTENTMANAGER_H
#define INTENTMANAGER_H

#include <ai/AIExport.h>
#include <ai/Intent.h>
#include <memory>
#include <string>
#include <vector>


namespace ai {

class AI_API IntentManager
{
public:
    IntentManager();
    ~IntentManager();

    void initialize();

    void registerIntent(std::shared_ptr<Intent> intent);
    std::shared_ptr<Intent> getIntent(const std::string &intentName) const;
    std::vector<std::shared_ptr<Intent>> getIntents() const;
    bool hasIntent(const std::string &intentName) const;
    void unregisterIntent(const std::string &intentName);

protected:
    struct Data;
    std::unique_ptr<Data> m_data;
}; // class IntentManager

} // namespace ai

#endif // INTENTMANAGER_H
