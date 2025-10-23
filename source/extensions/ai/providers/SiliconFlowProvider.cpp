#include "SiliconFlowProvider.h"
#include "SiliconFlowModel.h"
#include "db/DatabaseConnection.h"
#include <ai/Model.h>
#include <cstdint>
#include <db/DatabaseManager.h>
#include <kernel/Configuration.h>
#include <kernel/Logger.h>
#include <kernel/entities/Entity.h>
#include <kernel/entities/ModelInfo.h>

#include "SiliconFlowModelExecutors.h"

#include <httplib.h>
#include <memory>
#include <string>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <fmt/format.h>
#include <nlohmann/json.hpp>

SiliconFlowProvider::SiliconFlowProvider() : Provider() {}

SiliconFlowProvider::~SiliconFlowProvider() {}

std::vector<std::string>
    SiliconFlowProvider::fetchModelList(const std::map<std::string, std::string> &params) const
{
    httplib::Client cli(getBaseUrl());
    httplib::Headers headers;
    headers.emplace("Authorization", fmt::format("Bearer {}", getApiKey()));

    httplib::Params httpParams;
    // docs:
    // https://docs.siliconflow.cn/cn/api-reference/models/get-model-list#response-object
    // silicon flow available params:
    // type: text, image, audio, video
    // sub_type: chat, embedding, reranker, text-to-image, image-to-image,
    // speech-to-text, text-to-video
    for (auto pair : params)
    {
        httpParams.emplace(pair.first, pair.second);
    }
    auto res = cli.Get("/models", httpParams, headers);
    std::vector<std::string> modelList;
    if (res && res->status == 200)
    {
        using json = nlohmann::json;
        json j = json::parse(res->body);
        for (const auto &item : j["data"])
        {
            modelList.push_back(item["id"]);
        }
    }
    return modelList;
}

ai::Model::Ptr SiliconFlowProvider::createModel(const std::string &modelName) const
{
    if (auto model = getModel(modelName))
    {
        return model;
    }
    auto model =
        std::shared_ptr<SiliconFlowModel>(new SiliconFlowModel, SiliconFlowModel::deleteFunc);
    setupModel(modelName, model);
    return model;
}

void SiliconFlowProvider::setupModel(const std::string &modelName, ai::Model::Ptr model) const
{
    std::shared_ptr<SiliconFlowModel> siliconFlowModel =
        std::dynamic_pointer_cast<SiliconFlowModel>(model);
    if (!siliconFlowModel)
    {
        return;
    }
    auto &dbManager = DatabaseManager::getInstance();
    auto conn = dbManager.getModelInfoConnection("siliconflow");
    if (!conn)
    {
        return;
    }
    const std::string sql = fmt::format("name = '{}'", modelName);
    struct ThCondition : public DatabaseConnection::Condition
    {
        std::string m_sql;
        ThCondition(const std::string &sql) : m_sql(sql) {};
        std::string operator()() const override { return m_sql; }

    } condition(sql);
    std::vector<std::shared_ptr<Entity>> ents;
    bool res = conn->find("siliconflow", condition, ents);
    if (!res || ents.empty())
    {
        return;
    }
    auto info = std::dynamic_pointer_cast<siliconflow::ModelInfo>(ents[0]);
    if (!info)
    {
        return;
    }
    siliconFlowModel->m_status.isValid = !info->isDeprecated;
    siliconFlowModel->m_status.isActive = !info->isDeprecated;
    siliconFlowModel->m_params.maxTokens = info->maxTokens;
    siliconFlowModel->m_property.modelName = info->name;

#ifdef GA_DEBUG
    if (modelName == "Qwen/Qwen3-8B")
    {
        auto param = siliconFlowModel->getParams();
        param.enableThinking = true;
    }
#endif

    {
        siliconFlowModel->m_capabilityFlags = 0;
        auto type = info->type;
        if (type.find("chat") != std::string::npos)
        {
            siliconFlowModel->m_capabilityFlags |=
                (uint32_t)ai::Model::ModelCapabilityFlag::kSupportText2Text;
            siliconFlowModel->m_property.modelType = ai::Model::ModelType::kText;
            siliconFlowModel->m_executor = std::make_shared<siliconflow::Text2Text>(model, *this);
        }
        if (type.find("image") != std::string::npos)
        {
            siliconFlowModel->m_capabilityFlags |=
                (uint32_t)ai::Model::ModelCapabilityFlag::kSupportText2Image;
            siliconFlowModel->m_property.modelType = ai::Model::ModelType::kImage;
            siliconFlowModel->m_executor =
                std::make_shared<siliconflow::Text2Image>(siliconFlowModel, *this);
        }
        if (type.find("audio") != std::string::npos)
        {
            siliconFlowModel->m_capabilityFlags |=
                (uint32_t)ai::Model::ModelCapabilityFlag::kSupportSpeech2Text;
            siliconFlowModel->m_property.modelType = ai::Model::ModelType::kAudio;
            siliconFlowModel->m_executor =
                std::make_shared<siliconflow::Speech2Text>(siliconFlowModel, *this);
        }
        if (type.find("video") != std::string::npos)
        {
            siliconFlowModel->m_capabilityFlags |=
                (uint32_t)ai::Model::ModelCapabilityFlag::kSupportText2Video;
            siliconFlowModel->m_property.modelType = ai::Model::ModelType::kVideo;
            siliconFlowModel->m_executor =
                std::make_shared<siliconflow::Text2Video>(siliconFlowModel, *this);
        }
    }
}
