#pragma once
#include <memory>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Core
{
    /**
     * @brief Singleton logger for the whole application.
     *
     * Logger provides a centralized logging system for the entire application using spdlog.
     * It creates a single colored console logger instance that can be accessed from anywhere
     * in the application. The logger supports multiple log levels and colored output.
     */
    class Logger
    {
    public:
        /**
         * @brief Gets the singleton logger instance.
         * @return Shared pointer to the spdlog logger instance
         *
         * This method returns the same logger instance on every call, ensuring
         * consistent logging throughout the application. The logger is automatically
         * created on first access with colored console output.
         */
        static std::shared_ptr<spdlog::logger> &Get()
        {
            static std::shared_ptr<spdlog::logger> instance = spdlog::stdout_color_mt("VisionCraft");
            return instance;
        }
    };

} // namespace Core

/**
 * @brief Logs a trace-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for detailed diagnostic information, typically only of interest when diagnosing problems.
 */
#define LOG_TRACE(...) ::Core::Logger::Get()->trace(__VA_ARGS__)

/**
 * @brief Logs a debug-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for information that is diagnostically helpful to developers.
 */
#define LOG_DEBUG(...) ::Core::Logger::Get()->debug(__VA_ARGS__)

/**
 * @brief Logs an info-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for general information about application flow and state.
 */
#define LOG_INFO(...) ::Core::Logger::Get()->info(__VA_ARGS__)

/**
 * @brief Logs a warning-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for potentially harmful situations that don't prevent the application from continuing.
 */
#define LOG_WARN(...) ::Core::Logger::Get()->warn(__VA_ARGS__)

/**
 * @brief Logs an error-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for error events that might still allow the application to continue running.
 */
#define LOG_ERROR(...) ::Core::Logger::Get()->error(__VA_ARGS__)

/**
 * @brief Logs a critical-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for very severe error events that might cause the application to abort.
 */
#define LOG_CRITICAL(...) ::Core::Logger::Get()->critical(__VA_ARGS__)
