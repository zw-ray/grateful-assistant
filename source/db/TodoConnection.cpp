#include "TodoConnection.h"
#include <chrono>
#include <date/date.h>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <kernel/Configuration.h>
#include <kernel/Logger.h>
#include <kernel/entities/Todo.h>
#include <memory>
#include <sqlite3.h>
#include <sstream>
#include <typeindex>
#include <vector>

struct TodoConnection::Data
{
    sqlite3 *handle = nullptr;
};

void createTable(sqlite3 *handle) {

};

TodoConnection::TodoConnection() : m_data(nullptr)
{
    auto &config = Configuration::getInstance();
    const std::string appDir =
        std::get<std::string>(config.get("app.working_dir", Configuration::ConfigValueType(std::string("."))));
    std::filesystem::path appPath(appDir);
    auto dbPath = appPath / "db" / "todos.db";
    bool needCreateTable = false;
    if (!std::filesystem::exists(dbPath))
    {
        std::filesystem::create_directories(dbPath.parent_path());
        needCreateTable = true;
    }
    sqlite3 *handle = nullptr;
    int res = sqlite3_open_v2(dbPath.string().c_str(), &handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (res == SQLITE_OK)
    {
        m_data = std::make_unique<Data>();
        m_data->handle = handle;
        Logger::logInfo("Database opened successfully at: " + dbPath.string());

        if (needCreateTable)
        {
            const std::string createTableStmt = "CREATE TABLE IF NOT EXISTS todos ("
                                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                "title TEXT NOT NULL,"
                                                "content TEXT,"
                                                "due_date TEXT,"
                                                "reminder_time TEXT,"
                                                "priority INTEGER,"
                                                "status INTEGER,"
                                                "created_at TEXT,"
                                                "updated_at TEXT);";
            res = sqlite3_exec(handle, createTableStmt.c_str(), nullptr, nullptr, nullptr);
            if (res != SQLITE_OK)
            {
                Logger::logError("Failed to create todos table: " + std::string(sqlite3_errmsg(handle)));
                sqlite3_close(handle);
                return;
            }
        }
    }
    else
    {
        Logger::logError("Failed to open database: " + dbPath.string());
        sqlite3_close(handle);
    }
}

TodoConnection::~TodoConnection() {}

std::vector<std::string> TodoConnection::getTableNames() const
{
    if (m_data && m_data->handle != nullptr)
    {
        const std::string stmt = "SELECT name FROM sqlite_master WHERE type='table';";
        sqlite3_stmt *stmtHandle = nullptr;
        int res = sqlite3_prepare_v2(m_data->handle, stmt.c_str(), -1, &stmtHandle, nullptr);

        switch (res)
        {
        case SQLITE_OK:
        {
            std::vector<std::string> tableNames;
            while (sqlite3_step(stmtHandle) == SQLITE_ROW)
            {
                const char *tableNameChar = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 0));
                tableNames.push_back(tableNameChar);
                Logger::logInfo("Found table: " + std::string(tableNameChar));
            }
            sqlite3_finalize(stmtHandle);
            return tableNames;
        }
        break;
        default:
        {
            Logger::logError("SQL error: " +
                             std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
            return {};
        }
        break;
        }
    }
    Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                     "db path is not exist?");
    return {};
}

bool TodoConnection::insertIntoTable(const std::string &tableName, const Entity &entity)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::type_index typeIndex = typeid(entity);
    if (typeIndex != typeid(Todo))
    {
        Logger::logError("Unsupported entity type for TODOS table insertion.");
        return false;
    }
    const Todo &todo = static_cast<const Todo &>(entity);
    std::string insertStmt = "INSERT INTO " + tableName +
                             " (title, content, due_date, reminder_time, "
                             "priority, status, created_at, updated_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmtHandle = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, insertStmt.c_str(), -1, &stmtHandle, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_data->handle)));
        return false;
    }
    res = sqlite3_bind_parameter_count(stmtHandle);
    if (res != 8)
    {
        Logger::logError("Invalid number of parameters for insert statement.");
        sqlite3_finalize(stmtHandle);
        return false;
    }
    res = sqlite3_bind_text(stmtHandle, 1, todo.title.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    res = sqlite3_bind_text(stmtHandle, 2, todo.content.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    std::string dueTimeStr =
        fmt::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(todo.dueTime));
    res = sqlite3_bind_text(stmtHandle, 3, dueTimeStr.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind dueTime parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    std::string reminderTimeStr =
        fmt::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(todo.reminderTime));
    res = sqlite3_bind_text(stmtHandle, 4, reminderTimeStr.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    res = sqlite3_bind_int(stmtHandle, 5, static_cast<int>(todo.priority));
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    res = sqlite3_bind_int(stmtHandle, 6, static_cast<int>(todo.status));
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    std::string createdAtStr =
        fmt::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(todo.createdAt));
    res = sqlite3_bind_text(stmtHandle, 7, createdAtStr.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    std::string updatedAtStr =
        fmt::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(todo.updatedAt));
    res = sqlite3_bind_text(stmtHandle, 8, updatedAtStr.c_str(), -1, SQLITE_STATIC);
    if (res != SQLITE_OK)
    {
        Logger::logError("Failed to bind title parameter: " + std::string(sqlite3_errmsg(m_data->handle)));
        sqlite3_finalize(stmtHandle);
        return false;
    }

    res = sqlite3_step(stmtHandle);
    if (res != SQLITE_DONE)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    res = sqlite3_finalize(stmtHandle);
    if (res != SQLITE_OK)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        return false;
    }
    return true;
}

bool TodoConnection::getAll(const std::string &tableName, std::vector<std::shared_ptr<Entity>> &entities)
{
    if (!m_data)
    {
        Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::string selectStmt = "SELECT * FROM " + tableName + ";";
    sqlite3_stmt *stmtHandle = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, selectStmt.c_str(), -1, &stmtHandle, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        return false;
    }
    while (sqlite3_step(stmtHandle) == SQLITE_ROW)
    {
        Todo todo;
        todo.id = sqlite3_column_int(stmtHandle, 0);
        todo.title = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 1));
        todo.content = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 2));
        std::string dueTimeStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 3));
        date::sys_seconds tp;
        std::istringstream dueTimeSS(dueTimeStr);
        dueTimeSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.dueTime = tp;
        std::string reminderTimeStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 4));
        std::istringstream reminderTimeSS(reminderTimeStr);
        reminderTimeSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.reminderTime = tp;
        todo.priority = static_cast<Todo::Priority>(sqlite3_column_int(stmtHandle, 5));
        todo.status = static_cast<Todo::Status>(sqlite3_column_int(stmtHandle, 6));
        std::string createdAtStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 7));
        std::istringstream createdAtSS(createdAtStr);
        createdAtSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.createdAt = tp;
        std::string updatedAtStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 8));
        std::istringstream updatedAtSS(updatedAtStr);
        updatedAtSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.updatedAt = tp;
        entities.push_back(std::make_shared<Todo>(std::move(todo)));
    }
    sqlite3_finalize(stmtHandle);
    return true;
}

bool TodoConnection::find(const std::string &tableName, const Condition &condition,
                          std::vector<std::shared_ptr<Entity>> &entities)
{
    if (!m_data || !m_data->handle)
    {
        Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::string selectStmt = "SELECT * FROM " + tableName;
    if (!condition().empty())
    {
        selectStmt += " WHERE " + condition();
    }
    selectStmt += ";";
    Logger::logDebug("TodoConnection::find: {}", selectStmt);
    sqlite3_stmt *stmtHandle;
    int res = sqlite3_prepare_v2(m_data->handle, selectStmt.c_str(), -1, &stmtHandle, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        return false;
    }
    while (sqlite3_step(stmtHandle) == SQLITE_ROW)
    {
        Todo todo;
        todo.id = sqlite3_column_int(stmtHandle, 0);
        todo.title = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 1));
        todo.content = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 2));
        std::string dueTimeStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 3));
        date::sys_seconds tp;
        std::istringstream dueTimeSS(dueTimeStr);
        dueTimeSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.dueTime = tp;
        std::string reminderTimeStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 4));
        std::istringstream reminderTimeSS(reminderTimeStr);
        reminderTimeSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.reminderTime = tp;
        todo.priority = static_cast<Todo::Priority>(sqlite3_column_int(stmtHandle, 5));
        todo.status = static_cast<Todo::Status>(sqlite3_column_int(stmtHandle, 6));
        std::string createdAtStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 7));
        std::istringstream createdAtSS(createdAtStr);
        createdAtSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.createdAt = tp;
        std::string updatedAtStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtHandle, 8));
        std::istringstream updatedAtSS(updatedAtStr);
        updatedAtSS >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        todo.updatedAt = tp;
        entities.push_back(std::make_shared<Todo>(std::move(todo)));
    }
    sqlite3_finalize(stmtHandle);
    return true;
}

bool TodoConnection::update(const std::string &tableName, const Entity &entity, const Condition &condition)
{
    if (!m_data)
    {
        Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::type_index typeIndex = typeid(entity);
    if (typeIndex != typeid(Todo))
    {
        Logger::logError("Unsupported entity type for TODOS table insertion.");
        return false;
    }
    std::string updateStmt = "UPDATE " + tableName +
                             " SET title = ?, content = ?, "
                             "due_date = ?, reminder_time = ?, priority = ?, "
                             "status = ?, created_at = ?, updated_at = ? ";
    if (!condition().empty())
    {
        updateStmt += " WHERE " + condition();
    }
    updateStmt += ";";
    sqlite3_stmt *stmtHandle = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, updateStmt.c_str(), -1, &stmtHandle, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    auto todo = static_cast<const Todo &>(entity);
    sqlite3_bind_text(stmtHandle, 1, reinterpret_cast<const char *>(todo.title.c_str()), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtHandle, 2, reinterpret_cast<const char *>(todo.content.c_str()), -1, SQLITE_TRANSIENT);
    std::string dueTimeStr = fmt::format("{:%Y-%m-%d %H:%M:%S}", todo.dueTime);
    sqlite3_bind_text(stmtHandle, 3, reinterpret_cast<const char *>(dueTimeStr.c_str()), -1, SQLITE_TRANSIENT);
    std::string reminderTimeStr = fmt::format("{:%Y-%m-%d %H:%M:%S}", todo.reminderTime);
    sqlite3_bind_text(stmtHandle, 4, reinterpret_cast<const char *>(reminderTimeStr.c_str()), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmtHandle, 5, static_cast<int>(todo.priority));
    sqlite3_bind_int(stmtHandle, 6, static_cast<int>(todo.status));
    std::string createdAtStr = fmt::format("{:%Y-%m-%d %H:%M:%S}", todo.createdAt);
    sqlite3_bind_text(stmtHandle, 7, reinterpret_cast<const char *>(createdAtStr.c_str()), -1, SQLITE_TRANSIENT);
    std::string updatedAtStr = fmt::format("{:%Y-%m-%d %H:%M:%S}", todo.updatedAt);
    sqlite3_bind_text(stmtHandle, 8, reinterpret_cast<const char *>(updatedAtStr.c_str()), -1, SQLITE_TRANSIENT);
    if (sqlite3_bind_parameter_count(stmtHandle) != 8)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    if (sqlite3_step(stmtHandle) != SQLITE_DONE)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    sqlite3_reset(stmtHandle);
    sqlite3_finalize(stmtHandle);
    return true;
}

bool TodoConnection::remove(const std::string &tableName, const Condition &condition)
{
    if (!m_data)
    {
        Logger::logError("TodoConnection has not been initialized yet. Maybe the "
                         "db path is not exist?");
        return false;
    }
    std::string deleteStmt = "DELETE FROM " + tableName;
    if (!condition().empty())
    {
        deleteStmt += " WHERE " + condition();
    }
    deleteStmt += ";";
    sqlite3_stmt *stmtHandle = nullptr;
    int res = sqlite3_prepare_v2(m_data->handle, deleteStmt.c_str(), -1, &stmtHandle, nullptr);
    if (res != SQLITE_OK)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    if (sqlite3_step(stmtHandle) != SQLITE_DONE)
    {
        Logger::logError("SQL error: " + std::string(reinterpret_cast<const char *>(sqlite3_errmsg(m_data->handle))));
        sqlite3_finalize(stmtHandle);
        return false;
    }
    sqlite3_reset(stmtHandle);
    sqlite3_finalize(stmtHandle);
    return true;
}
