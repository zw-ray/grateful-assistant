/*******************************************************************************
**     FileName: ModelInfo.h
**    ClassName: ModelInfo
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 13:09
**  Description:
*******************************************************************************/

#ifndef MODELINFO_H
#define MODELINFO_H

#include <cstdint>
#include <kernel/entities/Entity.h>
#include <string>


namespace siliconflow {

struct ModelInfo : public Entity
{
    int32_t id;               // primary key, 自增ID
    std::string name;         // 模型名称，如："SiliconFlow"
    std::string type;         // 类型：chat/image/video/audio/embed/rerank
    std::string label;        // 标签：vision/moe/reason/tools/fim/math/coder/prefix
    std::string manufacturer; // 厂商：qwen/baichuan/llama/deepseek/...

    bool isDeprecated;  // 是否弃用，true表示弃用，false表示未弃用
    uint32_t maxTokens; // 最大token数，如：4096

    operator std::string() const override { return "siliconflow"; }
};
} // namespace siliconflow

// ... 其他模型信息结构体 ...

#endif // MODELINFO_H
