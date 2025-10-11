#include "Persistence/DockingLayoutHelper.h"
#include "Logger.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace VisionCraft
{
    void DockingLayoutHelper::SetupDefaultLayout()
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_left;
        ImGuiID dock_id_right;
        ImGuiID dock_id_top;

        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.075f, &dock_id_top, &dock_main_id);
        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, &dock_id_left, &dock_main_id);
        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, &dock_id_right, &dock_main_id);

        ImGui::DockBuilderDockWindow("Execution", dock_id_top);
        ImGui::DockBuilderDockWindow("Properties", dock_id_right);
        ImGui::DockBuilderDockWindow("Node Editor", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);

        LOG_INFO("DockingLayoutHelper: Default dock layout created");
    }
} // namespace VisionCraft
