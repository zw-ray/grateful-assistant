#include "AwakeWordVerifyRole.h"

#include <ai/AssistantRole.h>
#include <ai/Model.h>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/Logger.h>

#include <chrono>
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace ai {

AwakeWordVerifyRole::AwakeWordVerifyRole() : AssistantRole() { m_roleName = "AwakeWordVerifyRole"; }

AwakeWordVerifyRole::~AwakeWordVerifyRole() {}

std::string AwakeWordVerifyRole::getRoleName() const { return m_roleName; }

std::string AwakeWordVerifyRole::getAssistantRoleDescription() const { return "校验唤醒词"; }

std::string AwakeWordVerifyRole::handleRequest(Model::Ptr model, const std::string &request)
{
    if (model == nullptr)
    {
        Logger::logDebug("AwakeWordVerifyRole::handleRequest: model is null");
        return "";
    }
    std::string prompt = "";
    {
        // 校验唤醒词 - 从软件运行目录下的prompts目录下读取wake_word.promt文件，
        // 然后将request替换到prompt中的{{user_input}}位置
        std::ifstream file("prompts/wake_word.prompt");
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
    }
    auto param = model->getParams();
    param.enableThinking = false;
    param.enableStreaming = false;
    model->setParams(param);
    Model::ModelGenerateResult result = model->text2Text(prompt);
    if (result.isSuccess())
    {
        Logger::logInfo("AwakeWordVerifyRole::handleRequest: model generate result: {}",
                        result.response);
        using json = nlohmann::json;
        try
        {
            json j = json::parse(result.response);
            // 校验返回结果是否满足要求
            if (!j.contains("verify_result") || !j.contains("similarity") ||
                !j.contains("awake_word") || !j.contains("request"))
            {
                Logger::logError("AwakeWordVerifyRole::handleRequest: verify_result or "
                                 "similarity or awake_word or request not found in "
                                 "response: {}",
                                 result.response);
                return "fail";
            }
            const std::string requestPinyin = j["request"].get<std::string>();
            const std::string awakeWordPinyin = j["awake_word"].get<std::string>();
            if (requestPinyin.find(awakeWordPinyin) != std::string::npos)
            {
                SystemEvents::SystemMessageEvent event;
                event.time = std::chrono::system_clock::now().time_since_epoch().count();
                event.message = fmt::format(u8"唤醒词校验成功，唤醒词：{}，输入文本：{}",
                                            u8"小竹小竹", request);
                EventBus::getInstance().publish_async<SystemEvents::SystemMessageEvent>(event);
                return "success";
            }
            else
            {
                if (j["similarity"] > 0.8)
                {
                    return j["verify_result"];
                }
                else
                {
                    return "fail";
                }
            }
        }
        catch (const json::parse_error &e)
        {
            Logger::logError("AwakeWordVerifyRole::handleRequest: json parse error: {}", e.what());
            return "fail";
        }
    }
    return "fail";
}

bool AwakeWordVerifyRole::acceptIntent(Intent::Ptr intent) const
{
    if (intent == nullptr)
    {
        Logger::logDebug("AwakeWordVerifyRole::acceptIntent: intent is null");
        return false;
    }
    return intent->getName() == "AwakeWordVerifyIntent";
}

} // namespace ai
