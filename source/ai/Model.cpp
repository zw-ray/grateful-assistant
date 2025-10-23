#include <ai/Model.h>
#include <ai/Provider.h>

namespace ai {

// ======================= executors =======================

Model::ModelExecutor::ModelExecutor(std::shared_ptr<Model> model, const Provider &provider)
    : m_model(model), m_provider(provider)
{
}

Model::ModelExecutor::~ModelExecutor() {}

Model::ModelGenerateResult Model::ModelExecutor::text2Text(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult Model::ModelExecutor::text2Image(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult Model::ModelExecutor::image2Image(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult
    Model::ModelExecutor::speech2Text(const std::vector<int16_t> &audio) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult Model::ModelExecutor::textToSpeech(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult Model::ModelExecutor::text2Video(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

Model::ModelGenerateResult Model::ModelExecutor::text2TextStream(const std::string &prompt) const
{
    ModelGenerateResult result;
    result.error = "Method not implemented";
    return result;
}

// ======================= models =======================

Model::Model() : m_capabilityFlags(0), m_params{0, 0.0f, 0.0f, 0.0f, 0.0f, false, 0}
{
    m_status.isValid = false;
    m_status.isActive = false;
    m_property.modelType = ModelType::kUnknown;
    m_property.modelSubType = ModelSubType::kUnknown;
    m_property.modelName = "";
}

Model::~Model() {}

bool Model::isValid() const { return m_status.isValid; }

bool Model::isActive() const { return m_status.isActive; }

Model::ModelType Model::getModelType() const { return m_property.modelType; }

Model::ModelSubType Model::getModelSubType() const { return m_property.modelSubType; }

std::string Model::getModelName() const { return m_property.modelName; }

bool Model::supportThinking() const
{
    return m_capabilityFlags & static_cast<uint32_t>(ModelCapabilityFlag::kSupportThinking);
}

bool Model::supportTool() const
{
    return m_capabilityFlags & static_cast<uint32_t>(ModelCapabilityFlag::kSupportTool);
}

bool Model::supportStreaming() const
{
    return m_capabilityFlags & static_cast<uint32_t>(ModelCapabilityFlag::kSupportStreaming);
}

Model::ModelGenerateResult Model::text2Text(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->text2Text(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::text2Image(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->text2Image(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::image2Image(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->image2Image(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::speech2Text(const std::vector<int16_t> &audio) const
{
    if (m_executor)
    {
        return m_executor->speech2Text(audio);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::textToSpeech(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->textToSpeech(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::text2Video(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->text2Video(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelGenerateResult Model::text2TextStream(const std::string &prompt) const
{
    if (m_executor)
    {
        return m_executor->text2TextStream(prompt);
    }

    ModelGenerateResult result;
    result.error = "No executor available";
    return result;
}

Model::ModelParams Model::getParams() const { return m_params; }

Model &Model::setParams(const ModelParams &params)
{
    m_params = params;
    return *this;
}
} // namespace ai
