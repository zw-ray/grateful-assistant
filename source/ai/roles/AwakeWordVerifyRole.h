/*******************************************************************************
**     FileName: AwakeWordVerifyRole.h
**    ClassName: AwakeWordVerifyRole
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/05 21:41
**  Description:
*******************************************************************************/

#ifndef AWAKEWORDVERIFYROLE_H
#define AWAKEWORDVERIFYROLE_H

#include <ai/AssistantRole.h>
#include <ai/Intent.h>
#include <ai/Model.h>

namespace ai {

/**
 * @brief 唤醒词校验角色，用来将识别出的文本与唤醒词进行校验
 * 校验规则：<AwakeWord> <Request>
 * AwakeWord：系统唤醒词 - 小竹小竹
 * Request：音频识别出来的文本 - 例如：“小竹小竹 打开灯”
 * 提示词规则：
 * 将<AwakeWord> 和 <Request>转换成拼音，然后比较<Request>
 * 是否包含<AwakeWord>的拼音
 * 如果包含，则校验通过，否则校验失败
 * 大模型应该返回结果：
 * {
 *     "awake_word":"xiaozhuxiaozhu",
 *     "request":"xiaozhuxiaozhu dakaideng",
 *     "verify_result": "success",
 *     "similarity": 0.8
 * }
 */
class AwakeWordVerifyRole : public AssistantRole
{
public:
    AwakeWordVerifyRole();
    ~AwakeWordVerifyRole();

    std::string getRoleName() const override;
    std::string getAssistantRoleDescription() const override;

    /**
     * @brief 校验唤醒词
     *
     * @param model 大模型
     * @param request 音频识别出来的文本 - 例如：“小竹小竹 打开灯”
     * @return std::string 校验结果 - 例如：“success”
     */
    std::string handleRequest(Model::Ptr model, const std::string &request) override;

    bool acceptIntent(Intent::Ptr intent) const override;

protected:
}; // class AwakeWordVerifyRole
} // namespace ai
#endif // AWAKEWORDVERIFYROLE_H
