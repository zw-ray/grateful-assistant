#include <cstdint>
#include <filesystem>
#include <fstream>
#include <kernel/Configuration.h>
#include <kernel/Logger.h>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

std::vector<std::string> splitKey(const std::string &key)
{
    std::vector<std::string> subKeys;
    std::stringstream ss(key);
    std::string token;
    while (std::getline(ss, token, '.'))
    {
        subKeys.push_back(token);
    }
    return subKeys;
}

// ========================= ConfigurationNode =========================

using json = nlohmann::json;

// ========================= Configuration =========================

struct Configuration::Data
{
    json values;
};

Configuration::Configuration() : m_data(std::make_unique<Data>()) {}

Configuration::~Configuration() { saveToFile(); }

Configuration &Configuration::getInstance()
{
    static Configuration instance;
    return instance;
}

Configuration &Configuration::set(const std::string &key,
                                  const ConfigValueType &value)
{
    // Check if key is a JSON Pointer (starts with '/')
    if (!key.empty() && key[0] == '/')
    {
        // Use JSON Pointer to set nested values
        try
        {
            std::visit([&](auto &&arg)
                       { m_data->values[json::json_pointer(key)] = arg; },
                       value);
        }
        catch (const std::exception &)
        {
            // Invalid JSON Pointer, fallback to default behavior
            std::visit([&](auto &&arg) { m_data->values[key] = arg; }, value);
        }
    }
    else
    {
        // Use original dot notation for backward compatibility
        std::visit([&](auto &&arg) { m_data->values[key] = arg; }, value);
    }
    return *this;
}

uint32_t Configuration::arraySize(const std::string &key)
{
    if (!key.empty() && key[0] == '/')
    {
        try
        {
            auto &j = m_data->values.at(json::json_pointer(key));
            if (j.is_array())
            {
                return static_cast<uint32_t>(j.size());
            }
        }
        catch (const std::exception &e)
        {
            Logger::logError("Failed to get array size for key {}: {}", key,
                             e.what());
            return 0;
        }
    }
    else
    {
        if (!m_data->values.contains(key))
        {
            return 0;
        }
        auto &j = m_data->values[key];
        if (j.is_array())
        {
            return static_cast<uint32_t>(j.size());
        }
    }
    return 0;
}

void Configuration::push_back(const std::string &key)
{
    if (!m_data->values.contains(key))
    {
        m_data->values[key] = json::array();
    }
    auto &j = m_data->values[key];
    if (j.is_array())
    {
        j.push_back(json::object());
    }
}

bool Configuration::contains(const std::string &key)
{
    return m_data->values.contains(key);
}

Configuration::ConfigValueType
    Configuration::get(const std::string &key,
                       const ConfigValueType &defaultValue)
{
    // Check if key is a JSON Pointer (starts with '/')
    if (!key.empty() && key[0] == '/')
    {
        // Use JSON Pointer to access nested values
        try
        {
            if (!m_data->values.contains(json::json_pointer(key)))
            {
                return defaultValue;
            }

            auto &j = m_data->values.at(json::json_pointer(key));
            if (j.is_string())
            {
                return j.get<std::string>();
            }
            else if (j.is_number_integer())
            {
                return j.get<int32_t>();
            }
            else if (j.is_number_unsigned())
            {
                return j.get<uint32_t>();
            }
            else if (j.is_boolean())
            {
                return j.get<bool>();
            }
            else if (j.is_number_float())
            {
                return j.get<float>();
            }
            else if (j.is_array())
            {
                if (j.empty())
                {
                    return defaultValue;
                }
                if (j[0].is_string())
                {
                    return j.get<std::vector<std::string>>();
                }
                else if (j[0].is_number_integer())
                {
                    return j.get<std::vector<int32_t>>();
                }
                else if (j[0].is_number_unsigned())
                {
                    return j.get<std::vector<uint32_t>>();
                }
                else if (j[0].is_boolean())
                {
                    return j.get<std::vector<bool>>();
                }
                else if (j[0].is_number_float())
                {
                    return j.get<std::vector<double>>();
                }
            }
        }
        catch (const std::exception &)
        {
            // Invalid JSON Pointer, fallback to default dot notation
            // or return default value
            return defaultValue;
        }
    }
    else
    {
        // Use original dot notation for backward compatibility
        if (!m_data->values.contains(key))
        {
            return defaultValue;
        }

        auto &j = m_data->values[key];
        if (j.is_string())
        {
            return j.get<std::string>();
        }
        else if (j.is_number_integer())
        {
            return j.get<int32_t>();
        }
        else if (j.is_number_unsigned())
        {
            return j.get<uint32_t>();
        }
        else if (j.is_boolean())
        {
            return j.get<bool>();
        }
        else if (j.is_number_float())
        {
            return j.get<float>();
        }
        else if (j.is_array())
        {
            if (j.empty())
            {
                return defaultValue;
            }
            if (j[0].is_string())
            {
                return j.get<std::vector<std::string>>();
            }
            else if (j[0].is_number_integer())
            {
                return j.get<std::vector<int32_t>>();
            }
            else if (j[0].is_number_unsigned())
            {
                return j.get<std::vector<uint32_t>>();
            }
            else if (j[0].is_boolean())
            {
                return j.get<std::vector<bool>>();
            }
            else if (j[0].is_number_float())
            {
                return j.get<std::vector<double>>();
            }
        }
    }

    return defaultValue;
}

bool Configuration::loadFromFile(const std::string &path)
{
    std::filesystem::path p(path);
    if (path.empty())
    {
        p = "config/appconfig.json";
    }
    if (!std::filesystem::exists(p))
    {
        return false;
    }
    std::ifstream file(p);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string buffer = ss.str();
    json j = json::parse(buffer.data());
    if (!j.is_object())
    {
        return false;
    }
    m_data->values = j;
    return true;
}

bool Configuration::saveToFile(const std::string &path)
{
    std::filesystem::path p(path);
    if (path.empty())
    {
        p = "config/appconfig.json";
    }
    if (!std::filesystem::exists(p.parent_path()))
    {
        std::filesystem::create_directories(p.parent_path());
    }
    std::ofstream file(p);
    if (!file.is_open())
    {
        return false;
    }
    file << m_data->values.dump(4); // 4 空格缩进
    file.close();                   // 确保文件关闭

    return true;
}
