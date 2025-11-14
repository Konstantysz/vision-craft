#pragma once

#include <string>
#include <vector>

namespace VisionCraft::Editor::Persistence
{
    /**
     * @brief Manager for tracking recently opened files.
     */
    class RecentFilesManager
    {
    public:
        /**
         * @brief Constructs recent files manager.
         * @param stateFilePath Path to the state file (default: "recent_files.json")
         */
        explicit RecentFilesManager(const std::string &stateFilePath = "recent_files.json");

        /**
         * @brief Default destructor.
         */
        ~RecentFilesManager() = default;

        /**
         * @brief Loads recent files list from file.
         * @return Vector of recent file paths (most recent first)
         */
        [[nodiscard]] std::vector<std::string> Load() const;

        /**
         * @brief Saves recent files list to file.
         * @param recentFiles Recent files list to save
         * @return True if saved successfully, false otherwise
         */
        bool Save(const std::vector<std::string> &recentFiles) const;

        /**
         * @brief Adds a file to the recent files list.
         * @param filePath Path to the file
         */
        void AddFile(const std::string &filePath);

        /**
         * @brief Gets the list of recent files.
         * @return Vector of recent file paths (most recent first)
         */
        [[nodiscard]] const std::vector<std::string> &GetFiles() const;

        /**
         * @brief Clears the recent files list.
         */
        void Clear();

        static constexpr size_t MAX_RECENT_FILES = 10; ///< Maximum number of recent files to store

    private:
        std::string stateFilePath;                    ///< Path to the state file
        mutable std::vector<std::string> recentFiles; ///< Cached recent files
        mutable bool filesCached = false;             ///< Whether files have been cached
    };
} // namespace VisionCraft::Editor::Persistence
