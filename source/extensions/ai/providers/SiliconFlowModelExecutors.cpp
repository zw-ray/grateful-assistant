#include "SiliconFlowModelExecutors.h"
#include <ai/Provider.h>
#include <fmt/chrono.h>
#include <kernel/Configuration.h>
#include <kernel/EventBus.h>
#include <kernel/Events.h>
#include <kernel/Logger.h>

// #ifndef CPPHTTPLIB_OPENSSL_SUPPORT
// #define CPPHTTPLIB_OPENSSL_SUPPORT
// #endif
#include <httplib.h>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <nlohmann/json.hpp>

#ifdef GA_DEBUG
#include <fstream>
#endif

namespace siliconflow {

using json = nlohmann::json;

// ======================= text 2 text =======================

Text2Text::Text2Text(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Text2Text::~Text2Text() {}

ai::Model::ModelGenerateResult Text2Text::text2Text(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    httplib::Client client(m_provider.getBaseUrl());
    const std::string path = "/v1/chat/completions";
    httplib::Headers headers;
    headers.insert({"Content-Type", "application/json"});
    headers.insert({"Authorization", "Bearer " + m_provider.getApiKey()});

    // 构建请求体
    json body;
    body["model"] = m_model->getModelName();
    body["messages"] = json::array();
    // body["max_tokens"] = m_model->getParams().maxTokens; // 最大生成的token数
    body["stream"] = m_model->getParams().enableStreaming;         // 非流式模式
    body["enable_thinking"] = m_model->getParams().enableThinking; // 开启思考模式
    if (m_model->getParams().enableThinking)
    {
        // set thinking budget
        body["thinking_budget"] = m_model->getParams().thinkingBudget;
    }
    json messageObject;
    messageObject["role"] = "user";
    messageObject["content"] = prompt;
    body["messages"].push_back(messageObject);

    // 发送请求
    auto res = client.Post(path, headers, body.dump(), "application/json");
    if (res && res->status == 200)
    {
        json response = json::parse(res->body);
        if (response.contains("choices") && !response["choices"].empty() &&
            response["choices"][0].contains("message") &&
            response["choices"][0]["message"].contains("content"))
        {
            std::string assistant_reply = response["choices"][0]["message"]["content"];
            result.response = assistant_reply;
        }
        else
        {
            Logger::logError("Failed to parse response from API: {}", res->body);
            result.error = "Failed to parse response from API";
        }
    }
    else
    {
        std::string errMsg = fmt::format("Failed to send request to API: {}", res->body);
        Logger::logError("{}", errMsg);
        result.error = errMsg;
    }

    return result;
}

// ======================= text 2 image =======================

Text2Image::Text2Image(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Text2Image::~Text2Image() {}

ai::Model::ModelGenerateResult Text2Image::text2Image(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    // TODO: 实现文本到图像的具体逻辑
    result.error = "Text2Image method not implemented";
    return result;
}

// ======================= image 2 image =======================

Image2Image::Image2Image(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Image2Image::~Image2Image() {}

ai::Model::ModelGenerateResult Image2Image::image2Image(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    // TODO: 实现图像到图像的具体逻辑
    result.error = "Image2Image method not implemented";
    return result;
}

// ======================= speech 2 text =======================
Speech2Text::Speech2Text(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Speech2Text::~Speech2Text() {}

static void sendSpeechRecognitionResultReadyEvent(const std::string &result)
{
    AIEvents::SpeechRecognitionResultReadyEvent event;
    event.time = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now().time_since_epoch())
                     .count();
    event.result = result;
    auto &eventBus = EventBus::getInstance();
    eventBus.publish_async<AIEvents::SpeechRecognitionResultReadyEvent>(event);

    SystemEvents::SystemMessageEvent messageEvent;
    messageEvent.time = event.time;
    messageEvent.message = fmt::format("Speech Recognition Result: {}", result);
#ifdef GA_DEBUG
    eventBus.publish_async<SystemEvents::SystemMessageEvent>(messageEvent);
#endif
    Logger::logInfo("Speech Recognition Result: {}", result);
}

#ifdef GA_DEBUG
static std::string getOutputAudioFilePath()
{
    std::string tempAudioDir = "";
// 日志路径有以下几种：
// 1. 当前软件运行目录；---- 调试环境下
// 2. 用户主目录下的 .logs 文件夹；---- 正式环境下
#ifdef GA_DEBUG
    auto &config = Configuration::getInstance();
    auto appDir = std::get<std::string>(
        config.get("/app/working_dir", Configuration::ConfigValueType(std::string("."))));
    tempAudioDir = (std::filesystem::path(appDir) / "tempAudios").string();
#else
#if defined(_WIN32)
    tempAudioDir = getenv("USERPROFILE");
#elif defined(__linux__)
    tempAudioDir = getenv("HOME");
#elif defined(__APPLE__)
    tempAudioDir = getenv("HOME");
    tempAudioDir += "/Library/Logs";
#else
    tempAudioDir = "";
#endif
    if (!tempAudioDir.empty())
    {
        tempAudioDir += "./tempAudios";
    }
#endif
    auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
    auto now_truncated = std::chrono::floor<std::chrono::seconds>(now);
    const std::string dateTime = fmt::format("{:%Y_%m_%d_%H_%M_%S}", now_truncated);
    const std::string fileName = fmt::format("{}.wav", dateTime);
    if (!std::filesystem::exists(tempAudioDir))
    {
        std::filesystem::create_directories(tempAudioDir);
    }
    const std::string filePath = (std::filesystem::path(tempAudioDir) / fileName).string();
    return filePath;
}

std::vector<uint8_t> generateWavHeader(int sampleRate, int channels, int bitsPerSample,
                                       size_t dataSize)
{

    std::vector<uint8_t> header(44);
    size_t byteRate = sampleRate * channels * bitsPerSample / 8;
    size_t blockAlign = channels * bitsPerSample / 8;

    // RIFF chunk
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    uint32_t chunkSize = 36 + dataSize;
    memcpy(&header[4], &chunkSize, 4);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';

    // fmt subchunk
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    uint32_t fmtSize = 16;
    memcpy(&header[16], &fmtSize, 4);
    uint16_t audioFormat = 1; // PCM
    memcpy(&header[20], &audioFormat, 2);
    memcpy(&header[22], &channels, 2);
    memcpy(&header[24], &sampleRate, 4);
    memcpy(&header[28], &byteRate, 4);
    memcpy(&header[32], &blockAlign, 2);
    memcpy(&header[34], &bitsPerSample, 2);

    // data subchunk
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    memcpy(&header[40], &dataSize, 4);

    return header;
}

static void saveAudioData(const std::vector<int16_t> &audio, const std::string &outputAudioPath)
{
    std::vector<uint8_t> wavHeader =
        generateWavHeader(16000, 1, 16, audio.size() * sizeof(int16_t));

    std::ofstream audioFile(outputAudioPath, std::ios::binary);
    audioFile.write(reinterpret_cast<const char *>(wavHeader.data()), wavHeader.size());
    audioFile.write(reinterpret_cast<const char *>(audio.data()), audio.size() * sizeof(int16_t));
    audioFile.close();
}

#endif

ai::Model::ModelGenerateResult Speech2Text::speech2Text(const std::vector<int16_t> &audio) const
{
    ai::Model::ModelGenerateResult result;

    // 检查音频数据是否为空
    if (audio.empty())
    {
        result.error = "Input audio is empty";
        Logger::logError(result.error);
        sendSpeechRecognitionResultReadyEvent(result.error);
        return result;
    }

    // 初始化HTTP客户端
    httplib::Client client(m_provider.getBaseUrl());
    // client.set_max_timeout(20000); // 设置超时时间（根据需求调整）
    client.set_error_logger(
        [&client](const httplib::Error &err, const httplib::Request *req)
        {
            switch (err)
            {
            case httplib::Error::Connection:
            {
                Logger::logError("Connection error: {}", httplib::to_string(err));
            }
            break;
            case httplib::Error::BindIPAddress:
            {
                Logger::logError("Bind IP address error: {}", httplib::to_string(err));
            }
            break;
            default:
            {
                Logger::logError("Unknown error: {}", httplib::to_string(err));
            }
            break;
            }
        });

    const std::string path = "/v1/audio/transcriptions";

    // 设置请求头（仅保留Authorization，删除Content-Type）
    httplib::Headers headers;
    headers.insert({"Authorization", "Bearer " + m_provider.getApiKey()});
    // headers.insert({"Content-Type", "multipart/form-data"});

    // 将int16_t音频数据写入到文件，确定文件是否正常
    const std::string outputAudioPath = getOutputAudioFilePath();
    saveAudioData(audio, outputAudioPath);

    std::ifstream inFile(outputAudioPath, std::ios::binary);
    if (!inFile.is_open())
    {
        result.error = "Failed to open audio file for reading";
        Logger::logError(result.error);
        sendSpeechRecognitionResultReadyEvent(result.error);
        return result;
    }
    Logger::logInfo("Sending audio file to LLM for recognition: {}", outputAudioPath);
    std::string bufferStr((std::istreambuf_iterator<char>(inFile)),
                          std::istreambuf_iterator<char>());

    httplib::UploadFormDataItems items = {
        {
            "file",      // 表单字段名（需与API要求一致）
            bufferStr,   // 二进制音频数据
            "audio.wav", // 文件名（API可能校验此名称）
            "audio/wav"  // MIME类型（根据API要求调整，如audio/pcm）
        },
        // 模型参数（必填）
        {"model", m_model->getModelName()},
        // 可选参数：语言（如中文）
        // {"language", "zh-CN"}
        // 可添加其他参数：如response_format="text"（默认json）
    };

    // 发送POST请求
    auto res = client.Post(path, headers, items);

    // 处理网络错误（无响应）
    if (!res)
    {
        std::string error_msg = fmt::format("Request failed: {}", httplib::to_string(res.error()));
        result.error = error_msg;
        Logger::logError(error_msg);
        sendSpeechRecognitionResultReadyEvent(result.error);
        return result;
    }

    // 处理HTTP非200状态码
    if (res->status != 200)
    {
        auto j = json::parse(res->body);
        auto code = j["code"].get<int>();
        auto message = j["message"].get<std::string>();
        std::string error_msg = fmt::format("API returned non-200 status: {},"
                                            " code: {}, message: {}",
                                            res->status, code, message);
        result.error = error_msg;
        Logger::logError(error_msg);
        sendSpeechRecognitionResultReadyEvent(result.error);
        return result;
    }

    // 解析JSON响应（捕获可能的异常）
    try
    {
        nlohmann::json response = nlohmann::json::parse(res->body);

        // 检查响应中是否包含"text"字段
        if (response.contains("text") && response["text"].is_string())
        {
            result.response = response["text"].get<std::string>();
            if (result.response.empty())
            {
                result.error = "Empty response from API";
                Logger::logError(result.error);
                sendSpeechRecognitionResultReadyEvent(result.error);
                return result;
            }
            else
            {
                result.error = "";
                Logger::logInfo("Received response: {}", result.response);
            }
        }
        else
        {
            std::string error_msg = "API response missing 'text' field: " + res->body;
            result.error = error_msg;
            Logger::logError(error_msg);
        }
    }
    catch (const nlohmann::json::exception &e)
    {
        // 处理JSON解析错误
        std::string error_msg = "Failed to parse JSON response: " + std::string(e.what()) +
                                ", raw response: " + res->body;
        result.error = error_msg;
        Logger::logError(error_msg);
    }
    if (result.isSuccess())
    {
        sendSpeechRecognitionResultReadyEvent(result.response);
    }
    else
    {
        sendSpeechRecognitionResultReadyEvent(result.error);
    }
    return result;
}

// ======================= text 2 speech =======================
Text2Speech::Text2Speech(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Text2Speech::~Text2Speech() {}

ai::Model::ModelGenerateResult Text2Speech::textToSpeech(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    // TODO: 实现文本到语音的具体逻辑
    result.error = "Text2Speech method not implemented";
    return result;
}

// ======================= text 2 video =======================
Text2Video::Text2Video(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Text2Video::~Text2Video() {}

ai::Model::ModelGenerateResult Text2Video::text2Video(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    // TODO: 实现文本到视频的具体逻辑
    result.error = "Text2Video method not implemented";
    return result;
}

// ======================= text 2 text stream =======================
Text2TextStream::Text2TextStream(std::shared_ptr<ai::Model> model, const ai::Provider &provider)
    : ai::Model::ModelExecutor(model, provider)
{
}

Text2TextStream::~Text2TextStream() {}

ai::Model::ModelGenerateResult Text2TextStream::text2TextStream(const std::string &prompt) const
{
    ai::Model::ModelGenerateResult result;
    // TODO: 实现文本到文本流的具体逻辑
    result.error = "Text2TextStream method not implemented";
    return result;
}

} // namespace siliconflow
