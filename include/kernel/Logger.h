/*******************************************************************************
**     FileName: Logger.h
**    ClassName: Logger
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:09
**  Description:
*******************************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include <fmt/format.h>
#include <kernel/KernelExport.h>
#include <memory>
#include <string>
#include <vector>

class KERNEL_API Logger
{
    Logger();
    ~Logger();

public:
    enum LogLevel
    {
        kTrace,
        kDebug,
        kInfo,
        kWarning,
        kError,
        kFatal,
    };
    class IAppender
    {
    public:
        using Ptr = std::shared_ptr<IAppender>;
        virtual ~IAppender() = 0;
        virtual std::string name() const = 0;
        virtual void append(LogLevel level, const std::string &message) = 0;
        virtual void flush() = 0; // Added flush method for flushing the buffer
        virtual void setLogLevel(
            LogLevel level) = 0; // Added method to set log level for the
    };

public:
    static Logger &getInstance();

    template <typename... Args> void log(LogLevel level, Args... args)
    {
        std::string message = fmt::format(args...);
        for (auto &appender : m_appenders)
        {
            appender->append(level, message.c_str());
        }
    }

    template <typename... Args> static void logDebug(Args... args)
    {
        getInstance().log(kDebug, args...);
    }
    template <typename... Args> static void logInfo(Args... args)
    {
        getInstance().log(kInfo, args...);
    }
    template <typename... Args> static void logWarning(Args... args)
    {
        getInstance().log(kWarning, args...);
    }
    template <typename... Args> static void logError(Args... args)
    {
        getInstance().log(kError, args...);
    }
    template <typename... Args> static void logFatal(Args... args)
    {
        getInstance().log(kFatal, args...);
    }

    void addAppender(IAppender::Ptr appender);
    void removeAppender(IAppender::Ptr appender);
    void removeAppender(const std::string &name);
    void setLogLevel(LogLevel level);

protected:
    std::vector<IAppender::Ptr> m_appenders;
}; // class Logger

#endif // LOGGER_H
