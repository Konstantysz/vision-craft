#include "ContextMenuRenderer.h"

#include <algorithm>
#include <unordered_map>

namespace VisionCraft
{
    ContextMenuResult ContextMenuRenderer::Render(bool hasSelection)
    {
        ContextMenuResult result;

        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            // Delete operation (disabled when no selection)
            if (ImGui::MenuItem("Delete", nullptr, false, hasSelection))
            {
                result.action = ContextMenuResult::Action::DeleteNodes;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            // Nested "Add Node" submenu
            const auto selectedNodeType = RenderAddNodeSubmenu();
            if (!selectedNodeType.empty())
            {
                result.action = ContextMenuResult::Action::CreateNode;
                result.nodeType = selectedNodeType;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return result;
    }

    void ContextMenuRenderer::SetAvailableNodeTypes(const std::vector<NodeTypeInfo> &nodeTypes)
    {
        availableNodeTypes = nodeTypes;
    }

    void ContextMenuRenderer::Open()
    {
        ImGui::OpenPopup("NodeContextMenu");
    }

    std::string ContextMenuRenderer::RenderAddNodeSubmenu()
    {
        std::string selectedType;

        if (ImGui::BeginMenu("Add Node"))
        {
            // Group nodes by category
            std::unordered_map<std::string, std::vector<const NodeTypeInfo *>> categorizedNodes;
            for (const auto &nodeType : availableNodeTypes)
            {
                categorizedNodes[nodeType.category].push_back(&nodeType);
            }

            // Render nodes by category
            bool firstCategory = true;
            for (const auto &[category, nodes] : categorizedNodes)
            {
                if (!firstCategory)
                {
                    ImGui::Separator();
                }
                firstCategory = false;

                for (const auto *nodeInfo : nodes)
                {
                    if (ImGui::MenuItem(nodeInfo->displayName.c_str()))
                    {
                        selectedType = nodeInfo->typeId;
                    }
                }
            }

            ImGui::EndMenu();
        }

        return selectedType;
    }
} // namespace VisionCraft
