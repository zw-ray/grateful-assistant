/*******************************************************************************
**     FileName: Intent.h
**    ClassName: Intent
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/14 18:34
**  Description:
*******************************************************************************/

#ifndef INTENT_H
#define INTENT_H

#include <ai/AIExport.h>
#include <memory>
#include <string>

namespace ai {

class AI_API Intent
{
public:
    using Ptr = std::shared_ptr<Intent>;
    virtual ~Intent() = default;

    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;

protected:
    std::string m_name;
    std::string m_description;
}; // class Intent

} // namespace ai

#endif // INTENT_H
