#include "UI/Widgets/ContextMenuRenderer.h"

#include <algorithm>
#include <unordered_map>

namespace VisionCraft::UI::Widgets
{
    ContextMenuResult ContextMenuRenderer::Render(bool hasSelection, bool hasClipboardData)
    {
        ContextMenuResult result;

        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            // Copy operation (disabled when no selection)
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, hasSelection))
            {
                result.action = ContextMenuResult::Action::CopyNodes;
                ImGui::CloseCurrentPopup();
            }

            // Cut operation (disabled when no selection)
            if (ImGui::MenuItem("Cut", "Ctrl+X", false, hasSelection))
            {
                result.action = ContextMenuResult::Action::CutNodes;
                ImGui::CloseCurrentPopup();
            }

            // Paste operation (disabled when clipboard is empty)
            if (ImGui::MenuItem("Paste", "Ctrl+V", false, hasClipboardData))
            {
                result.action = ContextMenuResult::Action::PasteNodes;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            // Delete operation (disabled when no selection)
            if (ImGui::MenuItem("Delete", "Del", false, hasSelection))
            {
                result.action = ContextMenuResult::Action::DeleteNodes;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            // Nested "Add Nodes::Node" submenu
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

        if (ImGui::BeginMenu("Add Nodes::Node"))
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
} // namespace VisionCraft::UI::Widgets
