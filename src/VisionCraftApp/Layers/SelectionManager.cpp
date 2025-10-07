#include "SelectionManager.h"

#include <algorithm>

namespace VisionCraft
{
    bool SelectionManager::IsNodeSelected(Engine::NodeId nodeId) const
    {
        return selectedNodes.contains(nodeId);
    }

    bool SelectionManager::HasSelection() const
    {
        return !selectedNodes.empty();
    }

    size_t SelectionManager::GetSelectionCount() const
    {
        return selectedNodes.size();
    }

    const std::unordered_set<Engine::NodeId> &SelectionManager::GetSelectedNodes() const
    {
        return selectedNodes;
    }

    void SelectionManager::SelectNode(Engine::NodeId nodeId)
    {
        selectedNodes.clear();
        selectedNodes.insert(nodeId);
        primarySelection = nodeId;
    }

    void SelectionManager::ToggleNodeSelection(Engine::NodeId nodeId)
    {
        if (selectedNodes.contains(nodeId))
        {
            selectedNodes.erase(nodeId);
            if (primarySelection == nodeId)
            {
                primarySelection = selectedNodes.empty() ? -1 : *selectedNodes.begin();
            }
        }
        else
        {
            selectedNodes.insert(nodeId);
            primarySelection = nodeId;
        }
    }

    void SelectionManager::AddToSelection(Engine::NodeId nodeId)
    {
        selectedNodes.insert(nodeId);
        primarySelection = nodeId;
    }

    void SelectionManager::RemoveFromSelection(Engine::NodeId nodeId)
    {
        selectedNodes.erase(nodeId);
        if (primarySelection == nodeId)
        {
            primarySelection = selectedNodes.empty() ? -1 : *selectedNodes.begin();
        }
    }

    void SelectionManager::ClearSelection()
    {
        selectedNodes.clear();
        primarySelection = -1;
    }

    Engine::NodeId SelectionManager::GetPrimarySelection() const
    {
        return primarySelection;
    }

    void SelectionManager::StartBoxSelection(const ImVec2 &startPos)
    {
        isBoxSelecting = true;
        boxSelectStart = startPos;
        boxSelectEnd = startPos;
        ClearSelection();
    }

    void SelectionManager::UpdateBoxSelection(const ImVec2 &endPos)
    {
        boxSelectEnd = endPos;
    }

    void SelectionManager::EndBoxSelection()
    {
        isBoxSelecting = false;
        // Update primary selection if we have any
        if (!selectedNodes.empty())
        {
            primarySelection = *selectedNodes.begin();
        }
    }

    bool SelectionManager::IsBoxSelecting() const
    {
        return isBoxSelecting;
    }

    std::pair<ImVec2, ImVec2> SelectionManager::GetBoxSelectionBounds() const
    {
        const ImVec2 minPos(std::min(boxSelectStart.x, boxSelectEnd.x), std::min(boxSelectStart.y, boxSelectEnd.y));
        const ImVec2 maxPos(std::max(boxSelectStart.x, boxSelectEnd.x), std::max(boxSelectStart.y, boxSelectEnd.y));
        return { minPos, maxPos };
    }

    void SelectionManager::StartDrag(const ImVec2 &mousePos,
        const std::unordered_map<Engine::NodeId, ImVec2> &nodePositions)
    {
        isDragging = true;
        dragOffsets.clear();

        for (const auto nodeId : selectedNodes)
        {
            if (const auto it = nodePositions.find(nodeId); it != nodePositions.end())
            {
                const auto &nodeScreenPos = it->second;
                dragOffsets[nodeId] = ImVec2(mousePos.x - nodeScreenPos.x, mousePos.y - nodeScreenPos.y);
            }
        }
    }

    void SelectionManager::StopDrag()
    {
        isDragging = false;
        dragOffsets.clear();
    }

    bool SelectionManager::IsDragging() const
    {
        return isDragging;
    }

    ImVec2 SelectionManager::GetDragOffset(Engine::NodeId nodeId) const
    {
        if (const auto it = dragOffsets.find(nodeId); it != dragOffsets.end())
        {
            return it->second;
        }
        return ImVec2(0.0f, 0.0f);
    }

    const std::unordered_map<Engine::NodeId, ImVec2> &SelectionManager::GetDragOffsets() const
    {
        return dragOffsets;
    }
} // namespace VisionCraft
