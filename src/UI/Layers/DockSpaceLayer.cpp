#include "Layers/DockSpaceLayer.h"
#include "Events/FileOpenedEvent.h"
#include "Events/LoadGraphEvent.h"
#include "Events/NewGraphEvent.h"
#include "Events/SaveGraphEvent.h"
#include "Persistence/DockingLayoutHelper.h"
#include "Application.h"

#include <imgui.h>

namespace VisionCraft
{
    DockSpaceLayer::DockSpaceLayer() : recentFilesManager("recent_files.json")
    {
        Kappa::Application::Get().GetEventBus().Subscribe<FileOpenedEvent>(
            [this](const FileOpenedEvent &event) { recentFilesManager.AddFile(event.GetFilePath()); });
    }

    void DockSpaceLayer::OnEvent(Kappa::Event &event)
    {
    }

    void DockSpaceLayer::OnUpdate(float deltaTime)
    {
    }

    void DockSpaceLayer::OnRender()
    {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                            | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);

        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            if (isFirstFrame)
            {
                isFirstFrame = false;
                DockingLayoutHelper::SetupDefaultLayout();
            }
        }

        if (ImGui::BeginMenuBar())
        {
            RenderFileMenu();
            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    void DockSpaceLayer::RenderFileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                NewGraphEvent event;
                OnEvent(event);
            }

            if (ImGui::MenuItem("Open...", "Ctrl+O"))
            {
                LoadGraphEvent event;
                OnEvent(event);
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                SaveGraphEvent event;
                OnEvent(event);
            }

            ImGui::Separator();

            RenderRecentFilesMenu();

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // TODO: Signal application to close
            }
            ImGui::EndMenu();
        }
    }

    void DockSpaceLayer::RenderRecentFilesMenu()
    {
        if (ImGui::BeginMenu("Recent Files"))
        {
            const auto &recentFiles = recentFilesManager.GetFiles();

            if (recentFiles.empty())
            {
                ImGui::MenuItem("(No recent files)", nullptr, false, false);
            }
            else
            {
                for (const auto &filePath : recentFiles)
                {
                    size_t lastSlash = filePath.find_last_of("/\\");
                    std::string filename = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

                    if (ImGui::MenuItem(filename.c_str()))
                    {
                        LoadGraphFromFileEvent loadEvent(filePath);
                        Kappa::Application::Get().GetEventBus().Publish(loadEvent);
                    }

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", filePath.c_str());
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Clear Recent Files"))
                {
                    recentFilesManager.Clear();
                }
            }

            ImGui::EndMenu();
        }
    }
} // namespace VisionCraft
