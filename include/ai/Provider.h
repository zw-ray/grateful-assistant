/*******************************************************************************
**     FileName: Provider.h
**    ClassName: Provider
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 19:38
**  Description:
*******************************************************************************/

#ifndef PROVIDER_H
#define PROVIDER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ai/AIExport.h>
#include <ai/Model.h>

namespace ai {

class AI_API Provider
{
public:
    Provider();
    virtual ~Provider();

    using Ptr = std::shared_ptr<Provider>;

    virtual std::string getName() const; // should be implemented in derived class
    virtual Provider &setApiKey(const std::string &apiKey);
    virtual std::string getApiKey() const;

    virtual Provider &setBaseUrl(const std::string &baseUrl);
    virtual std::string getBaseUrl() const;

    virtual Provider &appendModel(Model::Ptr model);
    virtual std::vector<Model::Ptr> getModels() const;
    virtual Model::Ptr getModel(const std::string &modelName) const;
    virtual std::string getActiveModel() const;
    virtual Provider &setActiveModel(const std::string &modelName);

    virtual std::vector<std::string>
        fetchModelList(const std::map<std::string, std::string> &params)
            const; // should be implemented in derived class
    virtual Model::Ptr
        createModel(const std::string &modelName) const; // should be implemented in derived class
    virtual bool serializable() const;                   // should be implemented in derived class

protected:
    friend class ProviderManager;
    std::string m_apiKey;
    std::string m_baseUrl;
    std::string m_activeModel;
    std::string m_name;
    std::map<std::string, Model::Ptr> m_models;
};

} // namespace ai

#endif // PROVIDER_H
