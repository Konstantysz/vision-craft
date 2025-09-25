#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <source_location>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Core
{
    /**
     * @brief Modern C++20 wrapper around spdlog with type safety.
     *
     * Provides compile-time format string validation and automatic source location.
     */
    class Logger
    {
    public:
        /**
         * @brief Gets the singleton logger instance.
         * @return Reference to the default logger instance
         */
        static Logger &Get()
        {
            static Logger instance(spdlog::stdout_color_mt("VisionCraft"));
            return instance;
        }

        /**
         * @brief Type-safe trace logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Trace(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->trace(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Type-safe debug logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Debug(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->debug(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Type-safe info logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Info(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->info(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Type-safe warning logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Warn(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->warn(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Type-safe error logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Error(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->error(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Type-safe critical logging with automatic source location.
         * @tparam Args Format argument types
         * @param format Format string
         * @param args Format arguments
         */
        template<typename... Args> void Critical(const std::string &format, Args &&...args)
        {
            const auto loc = std::source_location::current();
            logger_->critical(
                "[{}:{}] {}", GetFileName(loc), loc.line(), std::vformat(format, std::make_format_args(args...)));
        }

        /**
         * @brief Direct access to underlying spdlog instance.
         * @return Reference to spdlog logger
         */
        spdlog::logger &GetSpdlog()
        {
            return *logger_;
        }
        const spdlog::logger &GetSpdlog() const
        {
            return *logger_;
        }

        /**
         * @brief Flushes the logger.
         */
        void Flush()
        {
            logger_->flush();
        }

        /**
         * @brief Sets the log level.
         * @param level spdlog log level
         */
        void SetLevel(spdlog::level::level_enum level)
        {
            logger_->set_level(level);
        }

    private:
        /**
         * @brief Private constructor for singleton pattern.
         * @param spdlogger spdlog logger instance
         */
        explicit Logger(std::shared_ptr<spdlog::logger> spdlogger) : logger_(std::move(spdlogger))
        {
        }

        /**
         * @brief Extracts filename from full path.
         * @param loc Source location
         * @return Just the filename without directory path
         */
        static std::string_view GetFileName(const std::source_location &loc)
        {
            const char *path = loc.file_name();
            const char *filename = path;

            for (const char *p = path; *p; ++p)
            {
                if (*p == '/' || *p == '\\')
                {
                    filename = p + 1;
                }
            }
            return filename;
        }

        std::shared_ptr<spdlog::logger> logger_;
    };

} // namespace Core

/**
 * @brief Logs a trace-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for detailed diagnostic information, typically only of interest when diagnosing problems.
 */
#define LOG_TRACE(...) ::Core::Logger::Get().Trace(__VA_ARGS__)

/**
 * @brief Logs a debug-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for information that is diagnostically helpful to developers.
 */
#define LOG_DEBUG(...) ::Core::Logger::Get().Debug(__VA_ARGS__)

/**
 * @brief Logs an info-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for general information about application flow and state.
 */
#define LOG_INFO(...) ::Core::Logger::Get().Info(__VA_ARGS__)

/**
 * @brief Logs a warning-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for potentially harmful situations that don't prevent the application from continuing.
 */
#define LOG_WARN(...) ::Core::Logger::Get().Warn(__VA_ARGS__)

/**
 * @brief Logs an error-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for error events that might still allow the application to continue running.
 */
#define LOG_ERROR(...) ::Core::Logger::Get().Error(__VA_ARGS__)

/**
 * @brief Logs a critical-level message.
 * @param ... Format string and arguments (printf-style)
 *
 * Use for very severe error events that might cause the application to abort.
 */
#define LOG_CRITICAL(...) ::Core::Logger::Get().Critical(__VA_ARGS__)
