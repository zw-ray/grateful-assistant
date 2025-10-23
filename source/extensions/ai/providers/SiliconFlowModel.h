/*******************************************************************************
**     FileName: SiliconFlowModel.h
**    ClassName: SiliconFlowModel
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 11:51
**  Description:
*******************************************************************************/

#ifndef SILICONFLOWMODEL_H
#define SILICONFLOWMODEL_H

#include <ai/Model.h>

class SiliconFlowModel : public ai::Model
{
protected:
    SiliconFlowModel() : Model() {}
    ~SiliconFlowModel() {}
    static void deleteFunc(SiliconFlowModel *model) { delete model; }
    friend class SiliconFlowProvider;

public:
    using Ptr = std::shared_ptr<SiliconFlowModel>;

}; // class SiliconFlowModel

#endif // SILICONFLOWMODEL_H
