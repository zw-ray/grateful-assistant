#include "SystemRole.h"
#include "ai/IntentManager.h"
#include "ai/RoleManager.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <kernel/Logger.h>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace ai {

struct SystemRole::Data
{
    RoleManager &roleManager;
    IntentManager &intentManager;
    Data(RoleManager &roleManager, IntentManager &intentManager)
        : roleManager(roleManager), intentManager(intentManager)
    {
    }
};

SystemRole::SystemRole(RoleManager &roleManager, IntentManager &intentManager)
    : AssistantRole(), m_data(std::make_unique<Data>(roleManager, intentManager))
{
    m_roleName = "SystemRole";
    m_roleDescription = "System role that handles system commands and intent recognition";
}

SystemRole::~SystemRole() {}

std::string SystemRole::getRoleName() const { return m_roleName; }

std::string SystemRole::getAssistantRoleDescription() const { return m_roleDescription; }

std::string SystemRole::handleRequest(Model::Ptr model, const std::string &request)
{
    std::string prompt = "";
    {
        // 意图识别 -
        // 从软件运行目录下的prompts目录下读取intent_recognition.prompt文件，
        // 然后将request替换到prompt中的{{user_input}}位置
        std::ifstream file("prompts/intent_recognition.prompt");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                prompt += line + "\n";
            }
            file.close();
        }
        else
        {
            Logger::logError("AwakeWordVerifyRole::handleRequest: wake_word.prompt file "
                             "not found");
            return "";
        }
        // 字符串替换
        while (prompt.find("{{user_input}}") != std::string::npos)
        {
            prompt.replace(prompt.find("{{user_input}}"), std::string("{{user_input}}").length(),
                           request);
        }

        // 从意图管理器中获取所有意图
        auto intents = m_data->intentManager.getIntents();
        // 构建意图列表
        std::string intentList = "";
        int intentIndex = 1;
        for (auto &intent : intents)
        {
            intentList += fmt::format("{}. {}: {}\n", intentIndex++, intent->getName(),
                                      intent->getDescription());
        }
        prompt += intentList;
        Logger::logDebug("SystemRole::handleRequest: prompt: {}", prompt);
    }

    using json = nlohmann::json;
    auto res = model->text2Text(prompt);
    auto j = json::parse(res.response);
    if (!j.contains("success") || !j.contains("intent"))
    {
        Logger::logError("SystemRole::handleRequest: Failed to parse JSON");
        return "Failed to parse JSON";
    }
    auto intent = j["intent"].get<std::string>();
    if (!j["success"])
    {
        Logger::logError("SystemRole::handleRequest: Failed to recognize "
                         "intent: \"{}\", for error: {}",
                         request, intent);
        return "Failed to recognize intent";
    }

    auto roles = m_data->roleManager.getRegisteredRoles();
    auto intentObj = m_data->intentManager.getIntent(intent);
    if (!intentObj)
    {
        Logger::logError("SystemRole::handleRequest: Failed to get intent \"{}\"", intent);
        return "Failed to get intent";
    }

    auto role = std::find_if(roles.begin(), roles.end(), [&intentObj](AssistantRole::Ptr role)
                             { return role->acceptIntent(intentObj); });
    if (role != roles.end())
    {
        return (*role)->handleRequest(model, request);
    }
    Logger::logError("SystemRole::handleRequest: Failed to create sub-role", intent);
    return "Failed to create sub-role";
}

bool SystemRole::acceptIntent(Intent::Ptr intent) const
{
    for (auto &pair : m_data->roleManager.getRegisteredRoles())
    {
        if (pair->acceptIntent(intent))
        {
            return true;
        }
    }
    return false;
}

} // namespace ai
