#include "UI/Widgets/FileDialogManager.h"

#include <ImGuiFileDialog.h>
#include <imgui.h>

namespace VisionCraft::UI::Widgets
{
    FileDialogResult FileDialogManager::RenderSaveDialog()
    {
        FileDialogResult result;

        // Open the dialog if requested
        if (shouldOpenSaveDialog && !ImGuiFileDialog::Instance()->IsOpened("SaveGraphDlgKey"))
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;

            ImGuiFileDialog::Instance()->OpenDialog(
                "SaveGraphDlgKey", "Save Graph", "Graph Files{.json,.JSON}", config);
            shouldOpenSaveDialog = false;
        }

        // Display and handle the dialog
        ImVec2 minSize = ImVec2(800, 500);
        ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

        if (ImGuiFileDialog::Instance()->Display("SaveGraphDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string selectedPath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (!selectedPath.empty())
                {
                    result.action = FileDialogResult::Action::Save;
                    result.filepath = EnsureJsonExtension(selectedPath);
                }
            }
            else
            {
                result.action = FileDialogResult::Action::Cancel;
            }

            ImGuiFileDialog::Instance()->Close();
        }

        return result;
    }

    FileDialogResult FileDialogManager::RenderLoadDialog()
    {
        FileDialogResult result;

        // Open the dialog if requested
        if (shouldOpenLoadDialog && !ImGuiFileDialog::Instance()->IsOpened("LoadGraphDlgKey"))
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            config.flags = ImGuiFileDialogFlags_Modal;

            ImGuiFileDialog::Instance()->OpenDialog(
                "LoadGraphDlgKey", "Load Graph", "Graph Files{.json,.JSON}", config);
            shouldOpenLoadDialog = false;
        }

        // Display and handle the dialog
        ImVec2 minSize = ImVec2(800, 500);
        ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

        if (ImGuiFileDialog::Instance()->Display("LoadGraphDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string selectedPath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (!selectedPath.empty())
                {
                    result.action = FileDialogResult::Action::Load;
                    result.filepath = selectedPath;
                }
            }
            else
            {
                result.action = FileDialogResult::Action::Cancel;
            }

            ImGuiFileDialog::Instance()->Close();
        }

        return result;
    }

    void FileDialogManager::OpenSaveDialog()
    {
        shouldOpenSaveDialog = true;
    }

    void FileDialogManager::OpenLoadDialog()
    {
        shouldOpenLoadDialog = true;
    }

    bool FileDialogManager::IsSaveDialogOpen() const
    {
        return ImGuiFileDialog::Instance()->IsOpened("SaveGraphDlgKey");
    }

    bool FileDialogManager::IsLoadDialogOpen() const
    {
        return ImGuiFileDialog::Instance()->IsOpened("LoadGraphDlgKey");
    }

    std::string FileDialogManager::EnsureJsonExtension(const std::string &filepath)
    {
        if (filepath.ends_with(".json") || filepath.ends_with(".JSON"))
        {
            return filepath;
        }
        return filepath + ".json";
    }
} // namespace VisionCraft::UI::Widgets
