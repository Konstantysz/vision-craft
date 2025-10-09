#include "Input/InputHandler.h"

#include "Editor/NodeEditorConstants.h"

namespace VisionCraft
{
    InputHandler::InputHandler(SelectionManager &selectionManager,
        ContextMenuRenderer &contextMenuRenderer,
        CanvasController &canvas)
        : selectionManager(selectionManager), contextMenuRenderer(contextMenuRenderer), canvas(canvas)
    {
    }

    std::vector<InputAction> InputHandler::ProcessInput(
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const PinId &hoveredPin,
        const FindNodeCallback &findNode,
        const FindConnectionCallback &findConnection,
        const UpdateBoxSelectionCallback &updateBoxSelection)
    {
        std::vector<InputAction> actions;

        auto &io = ImGui::GetIO();
        const auto mousePos = io.MousePos;

        // Handle Delete key
        if (const auto deleteAction = HandleDeleteKey())
        {
            actions.push_back(deleteAction.value());
            return actions;
        }

        // Early exit if not hovering window or panning
        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            InputAction action;
            action.type = InputActionType::UpdateHoveredConnection;
            action.hoveredConnection = std::nullopt;
            actions.push_back(action);
            hoveredConnection = std::nullopt;
            return actions;
        }

        // Update hovered connection
        actions.push_back(UpdateHoveredConnection(mousePos, hoveredPin, findConnection));

        // Handle right-click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            auto rightClickActions = HandleRightClick(mousePos, findNode);
            actions.insert(actions.end(), rightClickActions.begin(), rightClickActions.end());
        }

        // Handle left-click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (!(io.WantCaptureMouse && ImGui::IsAnyItemActive()))
            {
                HandleLeftClick(mousePos, nodePositions, findNode);
            }
        }

        // Handle mouse drag
        auto dragActions = HandleMouseDrag(mousePos, nodePositions, updateBoxSelection);
        actions.insert(actions.end(), dragActions.begin(), dragActions.end());

        // Handle mouse release
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            HandleMouseRelease();
        }

        return actions;
    }

    std::optional<InputAction> InputHandler::HandleDeleteKey()
    {
        if (selectionManager.HasSelection() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            InputAction action;
            action.type = InputActionType::DeleteNodes;
            action.nodeIds = std::vector<Engine::NodeId>(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            return action;
        }
        return std::nullopt;
    }

    std::vector<InputAction> InputHandler::HandleRightClick(const ImVec2 &mousePos, const FindNodeCallback &findNode)
    {
        std::vector<InputAction> actions;

        // First check if clicking on a connection
        if (hoveredConnection.has_value())
        {
            InputAction action;
            action.type = InputActionType::DeleteConnection;
            action.connection = hoveredConnection;
            actions.push_back(action);

            hoveredConnection = std::nullopt;
            return actions;
        }

        // Then check if clicking on a node
        const auto clickedNodeId = findNode(mousePos);
        if (clickedNodeId == Constants::Special::kInvalidNodeId)
        {
            // Right-click on empty space - clear selection and show creation menu
            selectionManager.ClearSelection();
            contextMenuPos = mousePos;
            contextMenuRenderer.Open();
        }
        else
        {
            // Right-click on node - select it and show context menu
            if (!selectionManager.IsNodeSelected(clickedNodeId))
            {
                selectionManager.SelectNode(clickedNodeId);
            }
            contextMenuRenderer.Open();
        }

        InputAction action;
        action.type = InputActionType::OpenContextMenu;
        action.contextMenuPos = mousePos;
        actions.push_back(action);

        return actions;
    }

    void InputHandler::HandleLeftClick(const ImVec2 &mousePos,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const FindNodeCallback &findNode)
    {
        const auto clickedNodeId = findNode(mousePos);
        const bool shiftPressed = ImGui::GetIO().KeyShift;

        if (clickedNodeId != Constants::Special::kInvalidNodeId)
        {
            // Clicked on a node
            if (shiftPressed)
            {
                // Shift+click: toggle selection
                selectionManager.ToggleNodeSelection(clickedNodeId);
            }
            else
            {
                // Normal click: select only this node (unless already in multi-selection)
                if (!selectionManager.IsNodeSelected(clickedNodeId))
                {
                    selectionManager.SelectNode(clickedNodeId);
                }
            }

            // Start dragging all selected nodes
            // Convert nodePositions to screen positions for drag calculation
            std::unordered_map<Engine::NodeId, ImVec2> screenPositions;
            for (const auto nodeId : selectionManager.GetSelectedNodes())
            {
                const auto &nodePos = nodePositions.at(nodeId);
                screenPositions[nodeId] = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
            }
            selectionManager.StartDrag(mousePos, screenPositions);
        }
        else
        {
            // Clicked on empty space
            if (!shiftPressed)
            {
                // Start box selection
                selectionManager.StartBoxSelection(mousePos);
            }
        }
    }

    std::vector<InputAction> InputHandler::HandleMouseDrag(const ImVec2 &mousePos,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const UpdateBoxSelectionCallback &updateBoxSelection)
    {
        std::vector<InputAction> actions;

        // Handle box selection
        if (selectionManager.IsBoxSelecting() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            selectionManager.UpdateBoxSelection(mousePos);
            updateBoxSelection();
        }

        // Handle node dragging
        if (selectionManager.IsDragging() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            InputAction action;
            action.type = InputActionType::UpdateNodePositions;

            const auto &dragOffsets = selectionManager.GetDragOffsets();
            for (const auto &[nodeId, offset] : dragOffsets)
            {
                const auto newWorldPos = ImVec2(mousePos.x - offset.x, mousePos.y - offset.y);
                const auto newNodePos = canvas.ScreenToWorld(newWorldPos);
                action.nodePositions[nodeId] = NodePosition{ newNodePos.x, newNodePos.y };
            }

            actions.push_back(action);
        }

        return actions;
    }

    void InputHandler::HandleMouseRelease()
    {
        if (selectionManager.IsBoxSelecting())
        {
            selectionManager.EndBoxSelection();
        }

        if (selectionManager.IsDragging())
        {
            selectionManager.StopDrag();
        }
    }

    InputAction InputHandler::UpdateHoveredConnection(const ImVec2 &mousePos,
        const PinId &hoveredPin,
        const FindConnectionCallback &findConnection)
    {
        InputAction action;
        action.type = InputActionType::UpdateHoveredConnection;

        // Update hovered connection for visual feedback (only if not dragging or interacting with pins)
        if (!selectionManager.IsDragging() && hoveredPin.nodeId == Constants::Special::kInvalidNodeId)
        {
            hoveredConnection = findConnection(mousePos);
            action.hoveredConnection = hoveredConnection;
        }
        else
        {
            hoveredConnection = std::nullopt;
            action.hoveredConnection = std::nullopt;
        }

        return action;
    }
} // namespace VisionCraft
