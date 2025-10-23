/*******************************************************************************
**     FileName: ProvidersExtension.h
**    ClassName: ProvidersExtension
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/14 20:17
**  Description:
*******************************************************************************/

#ifndef PROVIDERSEXTENSION_H
#define PROVIDERSEXTENSION_H

#include "OllamaProvider.h"
#include "SiliconFlowProvider.h"
#include <kernel/Extension.h>

class ProvidersExtension : public Extension
{
public:
    ProvidersExtension();
    ~ProvidersExtension();

    virtual void initialize() override;
    virtual void finalize() override;
    virtual std::string name() const override;
    virtual ExtensionInfo info() const override;

protected:
    std::shared_ptr<SiliconFlowProvider> m_siliconFlowProvider;
    std::shared_ptr<OllamaProvider> m_ollamaProvider;
}; // class ProvidersExtension

EXTENSION_EXPORT_FUNCTION()

#endif // PROVIDERSEXTENSION_H
