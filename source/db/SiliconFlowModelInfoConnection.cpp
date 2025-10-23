#include "SiliconFlowModelInfoConnection.h"
#include <filesystem>
#include <kernel/Configuration.h>
#include <kernel/Logger.h>
#include <kernel/entities/ModelInfo.h>
#include <memory>
#include <sqlite3.h>
#include <typeindex>
#include <vector>

struct SiliconFlowModelInfoConnection::Data
{
    sqlite3 *handle = nullptr;
};

SiliconFlowModelInfoConnection::SiliconFlowModelInfoConnection()
    : m_data(nullptr)
{
    auto &config = Configuration::getInstance();
    const std::string appDir = std::get<std::string>(config.get(
        "app.working_dir", Configuration::ConfigValueType(std::string("."))));
    std::filesystem::path appPath(appDir);
    auto dbPath = appPath / "db" / "model_infos.db";
    bool needCreateTable = false;
    if (!std::filesystem::exists(dbPath))
    {
        std::filesystem::create_directories(dbPath.parent_path());
        needCreateTable = true;
    }
#if defined(GA_DEBUG)
    else
    {
        static int enteredCount = 0;
        if (enteredCount++ == 0)
        {
            bool res = std::filesystem::remove(dbPath);
            if (!res)
            {
                Logger::logError("Failed to delete database: {}",
                                 dbPath.string());
                return;
            }
            needCreateTable = true;
        }
    }
#endif
    sqlite3 *handle = nullptr;
    int res =
        sqlite3_open_v2(dbPath.string().c_str(), &handle,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to open database: " +
                         std::string(sqlite3_errmsg(handle)));
        sqlite3_close(handle);
        return;
    }
    m_data = std::make_unique<Data>();
    m_data->handle = handle;
    if (needCreateTable)
    {
        const std::string createTableSql =
            "CREATE TABLE IF NOT EXISTS "
            "siliconflow (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT "
            "NULL, type TEXT NOT NULL, label TEXT NOT NULL, manufacturer TEXT "
            "NOT NULL, is_deprecated INTEGER NOT NULL, max_tokens INTEGER NOT "
            "NULL);";
        res = sqlite3_exec(handle, createTableSql.c_str(), nullptr, nullptr,
                           nullptr);
        if (res != SQLITE_OK)
        {
            Logger::logError("Failed to create table: " +
                             std::string(sqlite3_errmsg(handle)));
            sqlite3_close(handle);
            return;
        }

        // 创建几条默认数据

        // ----- 默认文本对话模型 -----
        siliconflow::ModelInfo Qwen3_235B_A22B_Instruct_2507;
        Qwen3_235B_A22B_Instruct_2507.name =
            "Qwen/Qwen3-235B-A22B-Instruct-2507";
        Qwen3_235B_A22B_Instruct_2507.type = "chat";
        Qwen3_235B_A22B_Instruct_2507.label = "prefix;tools;moe";
        Qwen3_235B_A22B_Instruct_2507.manufacturer = "Qwen";
        Qwen3_235B_A22B_Instruct_2507.isDeprecated = 0;
        Qwen3_235B_A22B_Instruct_2507.maxTokens = 4096 * 64;
        siliconflow::ModelInfo DeepSeek_R1_0528_Qwen3_8B;
        DeepSeek_R1_0528_Qwen3_8B.name =
            "deepseek-ai/DeepSeek-R1-0528-Qwen3-8B";
        DeepSeek_R1_0528_Qwen3_8B.type = "chat";
        DeepSeek_R1_0528_Qwen3_8B.label = "reason";
        DeepSeek_R1_0528_Qwen3_8B.manufacturer = "deepseek-ai";
        DeepSeek_R1_0528_Qwen3_8B.isDeprecated = 0;
        DeepSeek_R1_0528_Qwen3_8B.maxTokens = 4096 * 64;
        siliconflow::ModelInfo Qwen3_8B;
        Qwen3_8B.name = "Qwen/Qwen3-8B";
        Qwen3_8B.type = "chat";
        Qwen3_8B.label = "reason;tools;";
        Qwen3_8B.manufacturer = "Qwen";
        Qwen3_8B.isDeprecated = 0;
        Qwen3_8B.maxTokens = 4096 * 64;
        // ---- 默认语音识别模型 -----
        siliconflow::ModelInfo FunAudioLLM_SenseVoiceSmall;
        FunAudioLLM_SenseVoiceSmall.name = "FunAudioLLM/SenseVoiceSmall";
        FunAudioLLM_SenseVoiceSmall.type = "audio";
        FunAudioLLM_SenseVoiceSmall.label = "audio";
        FunAudioLLM_SenseVoiceSmall.manufacturer = "FunAudioLLM";
        FunAudioLLM_SenseVoiceSmall.isDeprecated = 0;
        FunAudioLLM_SenseVoiceSmall.maxTokens = 0;

        insertIntoTable("siliconflow", Qwen3_235B_A22B_Instruct_2507);
        insertIntoTable("siliconflow", DeepSeek_R1_0528_Qwen3_8B);
        insertIntoTable("siliconflow", Qwen3_8B);
        insertIntoTable("siliconflow", FunAudioLLM_SenseVoiceSmall);
    }
}

SiliconFlowModelInfoConnection::~SiliconFlowModelInfoConnection()
{
    if (m_data && m_data->handle)
    {
        sqlite3_close(m_data->handle);
        m_data->handle = nullptr;
    }
}

std::vector<std::string> SiliconFlowModelInfoConnection::getTableNames() const
{
    return {"siliconflow"};
}

bool SiliconFlowModelInfoConnection::insertIntoTable(
    const std::string &tableName, const Entity &entity)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("SiliconFlowModelInfoConnection has not been "
                         "initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::type_index typeIndex = typeid(entity);
    if (typeIndex != typeid(siliconflow::ModelInfo))
    {
        Logger::logError("Unsupported entity type for TODOS table insertion.");
        return false;
    }
    auto info = static_cast<const siliconflow::ModelInfo &>(entity);
    std::string insertSql =
        "INSERT INTO " + tableName +
        " (name, type, label, manufacturer, "
        "is_deprecated, max_tokens) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, insertSql.c_str(), -1, &stmt,
                                 nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_text(stmt, 1, info.name.c_str(), -1, SQLITE_TRANSIENT);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_text(stmt, 2, info.type.c_str(), -1, SQLITE_TRANSIENT);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_text(stmt, 3, info.label.c_str(), -1, SQLITE_TRANSIENT);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_text(stmt, 4, info.manufacturer.c_str(), -1,
                            SQLITE_TRANSIENT);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_int(stmt, 5, info.isDeprecated);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_bind_int(stmt, 6, info.maxTokens);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind parameter: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        Logger::logError("Failed to execute statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool SiliconFlowModelInfoConnection::getAll(
    const std::string &tableName,
    std::vector<std::shared_ptr<Entity>> &entities)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("SiliconFlowModelInfoConnection has not been "
                         "initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    const std::string selectSql = "SELECT * FROM " + tableName;
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, selectSql.c_str(), -1, &stmt,
                                 nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        siliconflow::ModelInfo info;
        info.id = sqlite3_column_int(stmt, 0);
        info.name =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        info.type =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        info.label =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        info.manufacturer =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        info.isDeprecated = sqlite3_column_int(stmt, 5);
        info.maxTokens = sqlite3_column_int(stmt, 6);
        entities.push_back(std::make_shared<siliconflow::ModelInfo>(info));
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return true;
}

bool SiliconFlowModelInfoConnection::find(
    const std::string &tableName, const Condition &condition,
    std::vector<std::shared_ptr<Entity>> &entities)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("SiliconFlowModelInfoConnection has not been "
                         "initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }

    std::string selectSql = "SELECT * FROM " + tableName;
    if (!condition().empty())
    {
        selectSql += " WHERE " + condition();
    }
    selectSql += ";";
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, selectSql.c_str(), -1, &stmt,
                                 nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        siliconflow::ModelInfo info;
        info.id = sqlite3_column_int(stmt, 0);
        info.name =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        info.type =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        info.label =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        info.manufacturer =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        info.isDeprecated = sqlite3_column_int(stmt, 5);
        info.maxTokens = sqlite3_column_int(stmt, 6);
        entities.push_back(
            std::make_shared<siliconflow::ModelInfo>(std::move(info)));
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return true;
}

bool SiliconFlowModelInfoConnection::update(const std::string &tableName,
                                            const Entity &entity,
                                            const Condition &condition)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("SiliconFlowModelInfoConnection has not been "
                         "initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    const auto &info = static_cast<const siliconflow::ModelInfo &>(entity);
    std::string updateSql = "UPDATE " + tableName + " SET " + "name = ?, " +
                            "type = ?, " + "label = ?, " +
                            "manufacturer = ?, " + "isDeprecated = ?, " +
                            "maxTokens = ? ";
    if (!condition().empty())
    {
        updateSql += " WHERE " + condition();
    }
    updateSql += ";";
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, updateSql.c_str(), -1, &stmt,
                                 nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_bind_text(stmt, 1, info.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, info.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, info.label.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, info.manufacturer.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, info.isDeprecated);
    sqlite3_bind_int(stmt, 6, info.maxTokens);
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        Logger::logError("Failed to execute statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return true;
}

bool SiliconFlowModelInfoConnection::remove(const std::string &tableName,
                                            const Condition &condition)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("SiliconFlowModelInfoConnection has not been "
                         "initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::string deleteSql = "DELETE FROM " + tableName;
    if (!condition().empty())
    {
        deleteSql += " WHERE " + condition();
    }
    deleteSql += ";";
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, deleteSql.c_str(), -1, &stmt,
                                 nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        Logger::logError("Failed to execute statement: " +
                         std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return true;
}
