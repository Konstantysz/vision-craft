#include "UI/Widgets/FileDialogManager.h"

#include <cstring>

#include <imgui.h>

namespace VisionCraft::UI::Widgets
{
    FileDialogResult FileDialogManager::RenderSaveDialog()
    {
        FileDialogResult result;

        if (!showSaveDialog)
        {
            return result;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Save Graph", &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filepath", filePathBuffer, sizeof(filePathBuffer));

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                std::string filepath = filePathBuffer;
                if (!filepath.empty())
                {
                    result.action = FileDialogResult::Action::Save;
                    result.filepath = EnsureJsonExtension(filepath);
                    showSaveDialog = false;
                    ClearBuffer();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                result.action = FileDialogResult::Action::Cancel;
                showSaveDialog = false;
                ClearBuffer();
            }

            ImGui::EndPopup();
        }

        return result;
    }

    FileDialogResult FileDialogManager::RenderLoadDialog()
    {
        FileDialogResult result;

        if (!showLoadDialog)
        {
            return result;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Load Graph", &showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filepath", filePathBuffer, sizeof(filePathBuffer));

            if (ImGui::Button("Load", ImVec2(120, 0)))
            {
                std::string filepath = filePathBuffer;
                if (!filepath.empty())
                {
                    result.action = FileDialogResult::Action::Load;
                    result.filepath = EnsureJsonExtension(filepath);
                    showLoadDialog = false;
                    ClearBuffer();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                result.action = FileDialogResult::Action::Cancel;
                showLoadDialog = false;
                ClearBuffer();
            }

            ImGui::EndPopup();
        }

        return result;
    }

    void FileDialogManager::OpenSaveDialog()
    {
        showSaveDialog = true;
        ImGui::OpenPopup("Save Graph");
    }

    void FileDialogManager::OpenLoadDialog()
    {
        showLoadDialog = true;
        ImGui::OpenPopup("Load Graph");
    }

    bool FileDialogManager::IsSaveDialogOpen() const
    {
        return showSaveDialog;
    }

    bool FileDialogManager::IsLoadDialogOpen() const
    {
        return showLoadDialog;
    }

    std::string FileDialogManager::EnsureJsonExtension(const std::string &filepath)
    {
        if (filepath.ends_with(".json"))
        {
            return filepath;
        }
        return filepath + ".json";
    }

    void FileDialogManager::ClearBuffer()
    {
        std::memset(filePathBuffer, 0, sizeof(filePathBuffer));
    }
} // namespace VisionCraft::UI::Widgets
