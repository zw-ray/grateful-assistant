/*******************************************************************************
**     FileName: OllamaProvider.h
**    ClassName: OllamaProvider
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 20:26
**  Description:
*******************************************************************************/

#ifndef OLLAMAPROVIDER_H
#define OLLAMAPROVIDER_H

#include <ai/Provider.h>

class OllamaProvider : public ai::Provider
{
public:
    OllamaProvider();
    ~OllamaProvider();

    std::string getName() const override { return "Ollama"; }

    std::vector<std::string>
        fetchModelList(const std::map<std::string, std::string> &params) const override;
    ai::Model::Ptr createModel(const std::string &modelName) const override;
    bool serializable() const override { return true; }

protected:
}; // class OllamaProvider

#endif // OLLAMAPROVIDER_H
