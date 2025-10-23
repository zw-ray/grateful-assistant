/*******************************************************************************
**     FileName: SiliconFlowModelExecutors.h
**    ClassName: SiliconFlowModelExecutors
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 11:14
**  Description:
*******************************************************************************/

#ifndef SILICONFLOWMODELEXECUTORS_H
#define SILICONFLOWMODELEXECUTORS_H

#include <ai/Model.h>
#include <ai/Provider.h>

namespace siliconflow {

class Text2Text : public ai::Model::ModelExecutor
{
public:
    Text2Text(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Text2Text() override;
    ai::Model::ModelGenerateResult text2Text(const std::string &prompt) const override;
};

class Text2Image : public ai::Model::ModelExecutor
{
public:
    Text2Image(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Text2Image() override;

    ai::Model::ModelGenerateResult text2Image(const std::string &prompt) const override;
};

class Image2Image : public ai::Model::ModelExecutor
{
public:
    Image2Image(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Image2Image() override;

    ai::Model::ModelGenerateResult image2Image(const std::string &prompt) const override;
};

class Speech2Text : public ai::Model::ModelExecutor
{
public:
    Speech2Text(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Speech2Text() override;

    ai::Model::ModelGenerateResult speech2Text(const std::vector<int16_t> &audio) const override;
};

class Text2Speech : public ai::Model::ModelExecutor
{
public:
    Text2Speech(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Text2Speech() override;

    ai::Model::ModelGenerateResult textToSpeech(const std::string &prompt) const override;
};

class Text2Video : public ai::Model::ModelExecutor
{
public:
    Text2Video(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Text2Video() override;
    ai::Model::ModelGenerateResult text2Video(const std::string &prompt) const override;
};

class Text2TextStream : public ai::Model::ModelExecutor
{
public:
    Text2TextStream(std::shared_ptr<ai::Model> model, const ai::Provider &provider);
    ~Text2TextStream() override;
    ai::Model::ModelGenerateResult text2TextStream(const std::string &prompt) const override;
};

} // namespace siliconflow

#endif // SILICONFLOWMODELEXECUTORS_H
