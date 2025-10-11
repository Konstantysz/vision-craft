#pragma once

#include "Event.h"
#include <string>

namespace VisionCraft
{
    /**
     * @brief Event emitted when a file is successfully opened or saved.
     * @note Used to track recent files.
     */
    class FileOpenedEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Constructs file opened event.
         * @param filePath Path to the file that was opened/saved
         */
        explicit FileOpenedEvent(std::string filePath) : filePath(std::move(filePath))
        {
        }

        /**
         * @brief Virtual destructor.
         */
        virtual ~FileOpenedEvent() = default;

        /**
         * @brief Gets the file path.
         * @return File path
         */
        [[nodiscard]] const std::string &GetFilePath() const
        {
            return filePath;
        }

    private:
        std::string filePath; ///< Path to the opened/saved file
    };
} // namespace VisionCraft
