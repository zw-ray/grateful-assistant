#include "kernel/Configuration.h"
#include <algorithm>
#include <filesystem>
#include <fmt/chrono.h>
#include <kernel/Logger.h>
#include <memory>
#include <stdarg.h>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// 实现 IAppender 接口的虚析构函数
Logger::IAppender::~IAppender() = default;

// 将自定义的 LogLevel 转换为 spdlog 的级别
spdlog::level::level_enum toSpdlogLevel(Logger::LogLevel level)
{
    switch (level)
    {
    case Logger::kTrace:
        return spdlog::level::trace;
    case Logger::kDebug:
        return spdlog::level::debug;
    case Logger::kInfo:
        return spdlog::level::info;
    case Logger::kWarning:
        return spdlog::level::warn;
    case Logger::kError:
        return spdlog::level::err;
    case Logger::kFatal:
        return spdlog::level::critical;
    default:
        return spdlog::level::info;
    }
}

// 基于 spdlog 的控制台输出器实现
class SpdlogConsoleAppender : public Logger::IAppender
{
public:
    SpdlogConsoleAppender() : m_level(Logger::kDebug)
    {
        m_logger = spdlog::stdout_color_mt("console");
#if defined(GA_DEBUG)
        m_logger->set_level(toSpdlogLevel(m_level));
#else
        m_logger->set_level(toSpdlogLevel(m_level));
#endif
    }

    ~SpdlogConsoleAppender() override = default;

    std::string name() const override { return "SpdlogConsoleAppender"; }

    void append(Logger::LogLevel level, const std::string &message) override
    {
        if (m_logger)
        {
            switch (level)
            {
            case Logger::kTrace:
                m_logger->trace(message);
                break;
            case Logger::kDebug:
                m_logger->debug(message);
                break;
            case Logger::kInfo:
                m_logger->info(message);
                break;
            case Logger::kWarning:
                m_logger->warn(message);
                break;
            case Logger::kError:
                m_logger->error(message);
                break;
            case Logger::kFatal:
                m_logger->critical(message);
                break;
            default:
                m_logger->info(message);
                break;
            }
        }
    }

    void flush() override { m_logger->flush(); }

    void setLogLevel(Logger::LogLevel level) override
    {
        m_level = level;
        m_logger->set_level(toSpdlogLevel(level));
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    Logger::LogLevel m_level;
};

static std::string getLoggerFilePath()
{
    std::string logDir = "";
// 日志路径有以下几种：
// 1. 当前软件运行目录；---- 调试环境下
// 2. 用户主目录下的 .logs 文件夹；---- 正式环境下
#ifdef GA_DEBUG
    auto &config = Configuration::getInstance();
    auto appDir = std::get<std::string>(config.get(
        "app.working_dir", Configuration::ConfigValueType(std::string(".")))
    );
    logDir = (std::filesystem::path(appDir) / "logs").string();
#else
#if defined(_WIN32)
    logDir = getenv("USERPROFILE");
#elif defined(__linux__)
    logDir = getenv("HOME");
#elif defined(__APPLE__)
    logDir = getenv("HOME");
    logDir += "/Library/Logs";
#else
    logDir = "";
#endif
    if (!logDir.empty())
    {
        logDir += "/.logs";
    }
#endif
    auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
    auto now_truncated = std::chrono::floor<std::chrono::seconds>(now);
    const std::string dateTime =
        fmt::format("{:%Y_%m_%d_%H_%M_%S}", now_truncated);
    const std::string fileName = fmt::format("{}.log", dateTime);
    if (!std::filesystem::exists(logDir))
    {
        std::filesystem::create_directories(logDir);
    }
    const std::string filePath =
        (std::filesystem::path(logDir) / fileName).string();
    return filePath;
}

// 基于 spdlog 的文件输出器实现
class SpdlogFileAppender : public Logger::IAppender
{
public:
    SpdlogFileAppender(const std::string &filename = getLoggerFilePath())
        : m_level(Logger::kDebug)
    {
        try
        {
            m_logger = spdlog::basic_logger_mt("file", filename);
#if defined(GA_DEBUG)
            m_logger->set_level(toSpdlogLevel(m_level));
#else
            m_logger->set_level(toSpdlogLevel(m_level));
#endif
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            // 如果文件创建失败，使用控制台日志记录错误
            spdlog::get("console")->error("Failed to create file logger: {}",
                                          ex.what());
        }
    }

    ~SpdlogFileAppender() override = default;

    std::string name() const override { return "SpdlogFileAppender"; }

    void append(Logger::LogLevel level, const std::string &message) override
    {
        if (m_logger)
        {
            switch (level)
            {
            case Logger::kTrace:
                m_logger->trace(message);
                break;
            case Logger::kDebug:
                m_logger->debug(message);
                break;
            case Logger::kInfo:
                m_logger->info(message);
                break;
            case Logger::kWarning:
                m_logger->warn(message);
                break;
            case Logger::kError:
                m_logger->error(message);
                break;
            case Logger::kFatal:
                m_logger->critical(message);
                break;
            default:
                m_logger->info(message);
            }
        }
        flush();
    }

    void flush() override
    {
        if (m_logger)
        {
            m_logger->flush();
        }
    }

    void setLogLevel(Logger::LogLevel level) override
    {
        m_level = level;
        if (m_logger)
        {
            m_logger->set_level(toSpdlogLevel(level));
        }
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    Logger::LogLevel m_level;
};

Logger::Logger()
{
    // 初始化默认的日志输出器
    // 先添加控制台输出器，因为文件输出器可能会用到它
    auto consoleAppender = std::make_shared<SpdlogConsoleAppender>();
    m_appenders.push_back(consoleAppender);

    // 添加文件输出器
    auto fileAppender = std::make_shared<SpdlogFileAppender>();
    m_appenders.push_back(fileAppender);
}

Logger::~Logger() = default;

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::addAppender(IAppender::Ptr appender)
{
    if (appender)
    {
        // 确保没有添加重复的appender
        auto it = std::find(m_appenders.begin(), m_appenders.end(), appender);
        if (it == m_appenders.end())
        {
            m_appenders.push_back(appender);
        }
    }
}

void Logger::removeAppender(IAppender::Ptr appender)
{
    if (!appender)
    {
        return;
    }

    auto it = std::remove(m_appenders.begin(), m_appenders.end(), appender);
    m_appenders.erase(it, m_appenders.end());
}

void Logger::removeAppender(const std::string &name)
{
    auto it = std::remove_if(m_appenders.begin(), m_appenders.end(),
                             [&name](const IAppender::Ptr &appender)
                             { return appender->name() == name; });
    m_appenders.erase(it, m_appenders.end());
}

void Logger::setLogLevel(LogLevel level)
{
    for (auto &appender : m_appenders)
    {
        appender->setLogLevel(level);
    }
}
