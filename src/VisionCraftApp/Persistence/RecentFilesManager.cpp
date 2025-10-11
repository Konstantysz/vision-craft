#include "Persistence/RecentFilesManager.h"
#include "Logger.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace VisionCraft
{
    RecentFilesManager::RecentFilesManager(const std::string &stateFilePath) : stateFilePath(stateFilePath)
    {
    }

    std::vector<std::string> RecentFilesManager::Load() const
    {
        std::vector<std::string> files;

        try
        {
            std::ifstream file(stateFilePath);
            if (!file.is_open())
            {
                LOG_INFO("RecentFilesManager: No recent files file found, starting with empty list");
                return files;
            }

            nlohmann::json json;
            file >> json;

            if (json.contains("recentFiles") && json["recentFiles"].is_array())
            {
                files = json["recentFiles"].get<std::vector<std::string>>();
            }

            LOG_INFO("RecentFilesManager: Loaded {} recent file(s) from '{}'", files.size(), stateFilePath);
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("RecentFilesManager: Failed to load recent files from '{}': {}", stateFilePath, e.what());
        }

        return files;
    }

    bool RecentFilesManager::Save(const std::vector<std::string> &files) const
    {
        try
        {
            nlohmann::json json;
            json["recentFiles"] = files;

            std::ofstream file(stateFilePath);
            if (!file.is_open())
            {
                LOG_ERROR("RecentFilesManager: Failed to open '{}' for writing", stateFilePath);
                return false;
            }

            file << json.dump(4);
            LOG_INFO("RecentFilesManager: Saved {} recent file(s) to '{}'", files.size(), stateFilePath);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("RecentFilesManager: Failed to save recent files to '{}': {}", stateFilePath, e.what());
            return false;
        }
    }

    void RecentFilesManager::AddFile(const std::string &filePath)
    {
        if (!filesCached)
        {
            recentFiles = Load();
            filesCached = true;
        }

        recentFiles.erase(std::remove(recentFiles.begin(), recentFiles.end(), filePath), recentFiles.end());

        recentFiles.insert(recentFiles.begin(), filePath);

        if (recentFiles.size() > MAX_RECENT_FILES)
        {
            recentFiles.resize(MAX_RECENT_FILES);
        }

        Save(recentFiles);
    }

    const std::vector<std::string> &RecentFilesManager::GetFiles() const
    {
        if (!filesCached)
        {
            recentFiles = Load();
            filesCached = true;
        }
        return recentFiles;
    }

    void RecentFilesManager::Clear()
    {
        recentFiles.clear();
        filesCached = true;
        Save(recentFiles);
    }
} // namespace VisionCraft
