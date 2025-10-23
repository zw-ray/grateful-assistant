#include "OllamaProvider.h"
#include <fmt/format.h>
#include <kernel/Configuration.h>
#include <kernel/Logger.h>
#include <nlohmann/json.hpp>
#include <vector>

#include <httplib.h>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

OllamaProvider::OllamaProvider() {}

OllamaProvider::~OllamaProvider() {}

std::vector<std::string>
    OllamaProvider::fetchModelList(const std::map<std::string, std::string> &params) const
{
    httplib::Client cli(getBaseUrl());
    httplib::Headers headers;
    headers.emplace("Authorization", fmt::format("Bearer {}", getApiKey()));

    auto res = cli.Get("/tags", headers);
    std::vector<std::string> modelList;
    if (res && res->status == 200)
    {
        using json = nlohmann::json;
        json j = json::parse(res->body);
        for (const auto &item : j["models"])
        {
            modelList.push_back(item["name"]);
        }
    }

    return modelList;
}

ai::Model::Ptr OllamaProvider::createModel(const std::string &modelName) const
{
    Logger::logWarning("OllamaProvider::createModel is not implemented");
    return nullptr;
}
