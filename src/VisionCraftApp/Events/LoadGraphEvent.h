#pragma once

#include "Event.h"
#include <string>

namespace VisionCraft
{
    /**
     * @brief Event for triggering graph load (via file dialog).
     */
    class LoadGraphEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Default constructor.
         */
        LoadGraphEvent() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~LoadGraphEvent() = default;
    };

    /**
     * @brief Event for loading a specific graph file.
     * @note Used when loading from recent files menu.
     */
    class LoadGraphFromFileEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Constructs load graph from file event.
         * @param filePath Path to the graph file to load
         */
        explicit LoadGraphFromFileEvent(std::string filePath) : filePath(std::move(filePath))
        {
        }

        /**
         * @brief Virtual destructor.
         */
        virtual ~LoadGraphFromFileEvent() = default;

        /**
         * @brief Gets the file path.
         * @return File path
         */
        [[nodiscard]] const std::string &GetFilePath() const
        {
            return filePath;
        }

    private:
        std::string filePath; ///< Path to the graph file
    };
} // namespace VisionCraft
