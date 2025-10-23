/*******************************************************************************
**     FileName: SiliconFlowProvider.h
**    ClassName: SiliconFlowProvider
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 20:16
**  Description:
*******************************************************************************/

#ifndef SILICONFLOWPROVIDER_H
#define SILICONFLOWPROVIDER_H

#include <ai/Model.h>
#include <ai/Provider.h>
#include <map>
#include <memory>
#include <vector>

class SiliconFlowProvider : public ai::Provider
{
public:
    using Ptr = std::shared_ptr<SiliconFlowProvider>;
    SiliconFlowProvider();
    ~SiliconFlowProvider();

    std::string getName() const override { return "SiliconFlow"; }

    std::vector<std::string>
        fetchModelList(const std::map<std::string, std::string> &params) const override;
    ai::Model::Ptr createModel(const std::string &modelName) const override;
    bool serializable() const override { return true; }

protected:
    void setupModel(const std::string &modelName, ai::Model::Ptr model) const;
}; // class SiliconFlowProvider

#endif // SILICONFLOWPROVIDER_H
