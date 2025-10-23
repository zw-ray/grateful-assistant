#include "ai/Model.h"
#include <ai/Provider.h>
#include <vector>

namespace ai {

Provider::Provider() : m_apiKey(""), m_baseUrl("") {}

Provider::~Provider() {}

std::string Provider::getName() const { return m_name; }

Provider &Provider::setApiKey(const std::string &apiKey)
{
    m_apiKey = apiKey;
    return *this;
}

std::string Provider::getApiKey() const { return m_apiKey; }

Provider &Provider::setBaseUrl(const std::string &baseUrl)
{
    m_baseUrl = baseUrl;
    return *this;
}

std::string Provider::getBaseUrl() const { return m_baseUrl; }

Provider &Provider::appendModel(Model::Ptr model)
{
    m_models[model->getModelName()] = model;
    return *this;
}

std::vector<Model::Ptr> Provider::getModels() const
{
    std::vector<Model::Ptr> models;
    for (const auto &pair : m_models)
    {
        models.push_back(pair.second);
    }
    return models;
}

Model::Ptr Provider::getModel(const std::string &modelName) const
{
    auto iter = m_models.find(modelName);
    if (iter == m_models.end())
    {
        return nullptr;
    }
    return iter->second;
}

std::string Provider::getActiveModel() const { return m_activeModel; }

Provider &Provider::setActiveModel(const std::string &activeModel)
{
    m_activeModel = activeModel;
    return *this;
}

std::vector<std::string>
    Provider::fetchModelList(const std::map<std::string, std::string> &params) const
{
    return {};
}

Model::Ptr Provider::createModel(const std::string &modelName) const { return nullptr; }

bool Provider::serializable() const { return false; }

} // namespace ai
