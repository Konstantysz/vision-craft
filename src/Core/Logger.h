#pragma once
#include <memory>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Core
{
    /**
     * @brief Singleton logger for the whole application.
     */
    class Logger
    {
    public:
        /**
         * @brief Get the singleton logger instance.
         * @return Shared pointer to the spdlog logger.
         */
        static std::shared_ptr<spdlog::logger> &Get()
        {
            static std::shared_ptr<spdlog::logger> instance = spdlog::stdout_color_mt("VisionCraft");
            return instance;
        }
    };

} // namespace Core

// Logging macros for different log levels
#define LOG_TRACE(...) ::Core::Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::Core::Logger::Get()->debug(__VA_ARGS__)
#define LOG_INFO(...) ::Core::Logger::Get()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Core::Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Core::Logger::Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Core::Logger::Get()->critical(__VA_ARGS__)
