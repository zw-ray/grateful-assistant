/*******************************************************************************
**     FileName: Configuration.h
**    ClassName: Configuration
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:13
**  Description: 软件配置类
*******************************************************************************/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>
#include <memory>
#include <string>

#include <kernel/KernelExport.h>
#include <variant>
#include <vector>

class KERNEL_API Configuration
{
    Configuration();
    ~Configuration();

public:
    static Configuration &getInstance();

    using ConfigValueType =
        std::variant<std::string, int32_t, uint32_t, bool, double,
                     std::vector<std::string>, std::vector<int32_t>,
                     std::vector<uint32_t>, std::vector<bool>,
                     std::vector<double>>;

    // 设置配置项
    Configuration &set(const std::string &key, const ConfigValueType &value);
    uint32_t arraySize(const std::string &key);
    void push_back(const std::string &key);
    bool contains(const std::string &key);

    // 获取配置项
    ConfigValueType get(const std::string &key,
                        const ConfigValueType &defaultValue);

    // 从文件加载配置
    bool loadFromFile(const std::string &path = "");

    // 保存配置到文件
    bool saveToFile(const std::string &path = "");

private:
    struct Data;
    std::unique_ptr<Data> m_data;
};

#endif // CONFIGURATION_H
