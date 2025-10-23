/*******************************************************************************
**     FileName: Model.h
**    ClassName: Model
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 10:39
**  Description:
*******************************************************************************/

#ifndef MODEL_H
#define MODEL_H

#include <ai/AIExport.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ai {

class Provider;

class AI_API Model
{
protected:
    Model();
    virtual ~Model();

public:
    using Ptr = std::shared_ptr<Model>;

    enum class ModelType
    {
        kUnknown = 0,
        kText,
        kImage,
        kAudio,
        kVideo,
    };

    enum class ModelSubType
    {
        kUnknown = 0,
        kChat,
        kEmbedding,
        kReranker,
        kTextToImage,
        kImageToImage,
        kSpeechToText,
        kTextToVideo
    };

    bool isValid() const;
    bool isActive() const;
    ModelType getModelType() const;
    ModelSubType getModelSubType() const;
    std::string getModelName() const;

    bool supportThinking() const;
    bool supportTool() const;
    bool supportStreaming() const;

    struct AI_API ModelGenerateResult
    {
        std::string response;
        std::vector<char> binary;
        bool isStreaming{false};
        bool isThinking{false};

        bool isSuccess() const { return error.empty(); }
        std::string error;
    };
    virtual ModelGenerateResult text2Text(const std::string &prompt) const;
    virtual ModelGenerateResult text2Image(const std::string &prompt) const;
    virtual ModelGenerateResult image2Image(const std::string &prompt) const;
    virtual ModelGenerateResult speech2Text(const std::vector<int16_t> &audio) const;
    virtual ModelGenerateResult textToSpeech(const std::string &prompt) const;
    virtual ModelGenerateResult text2Video(const std::string &prompt) const;
    virtual ModelGenerateResult text2TextStream(const std::string &prompt) const;

    class AI_API ModelExecutor
    {
    public:
        using Ptr = std::shared_ptr<ModelExecutor>;
        ModelExecutor(std::shared_ptr<Model> model, const Provider &provider);
        virtual ~ModelExecutor();

        virtual ModelGenerateResult text2Text(const std::string &prompt) const;
        virtual ModelGenerateResult text2Image(const std::string &prompt) const;
        virtual ModelGenerateResult image2Image(const std::string &prompt) const;
        virtual ModelGenerateResult speech2Text(const std::vector<int16_t> &audio) const;
        virtual ModelGenerateResult textToSpeech(const std::string &prompt) const;
        virtual ModelGenerateResult text2Video(const std::string &prompt) const;
        virtual ModelGenerateResult text2TextStream(const std::string &prompt) const;

    protected:
        std::shared_ptr<Model> m_model;
        const Provider &m_provider;
    };

    struct AI_API ModelParams
    {
        uint32_t maxTokens;      // max tokens to generate
        float temperature;       // temperature of the model
        float topP;              // top p of the model
        float topK;              // top k of the model
        float repetitionPenalty; // repetition penalty of the model

        // thinking settings
        bool enableThinking;     // enable thinking of the model
        uint32_t thinkingBudget; // thinking budget of the model
        // streaming settings
        bool enableStreaming; // enable streaming of the model
    };
    ModelParams getParams() const;
    Model &setParams(const ModelParams &params);

    enum class ModelCapabilityFlag : uint32_t
    {
        kSupportThinking = 0x01,         // 支持思考
        kSupportTool = 0x02,             // 支持工具
        kSupportStreaming = 0x04,        // 支持流式
        kSupportText2Text = 0x08,        // 支持文本到文本
        kSupportText2Image = 0x10,       // 支持文本到图片
        kSupportImage2Image = 0x20,      // 支持图片到图片
        kSupportSpeech2Text = 0x40,      // 支持语音到文本
        kSupportTextToSpeech = 0x80,     // 支持文本到语音
        kSupportText2Video = 0x100,      // 支持文本到视频
        kSupportText2TextStream = 0x200, // 支持文本到文本流式
    };

protected:
    struct ModelStatus
    {
        bool isValid;
        bool isActive;
    } m_status;
    uint32_t m_capabilityFlags;
    struct ModelProperty
    {
        ModelType modelType;
        ModelSubType modelSubType;
        std::string modelName; // name or id of the model
    } m_property;
    std::shared_ptr<Provider> m_provider;
    std::shared_ptr<ModelExecutor> m_executor;
    ModelParams m_params;
}; // class Model'

} // namespace ai
#endif // MODEL_H
