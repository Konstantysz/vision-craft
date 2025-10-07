#pragma once

#include "NodeEditorTypes.h"

#include <unordered_set>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Manages node selection state and operations.
     *
     * Encapsulates all selection-related state including single selection,
     * multi-selection, box selection, and drag state.
     */
    class SelectionManager
    {
    public:
        /**
         * @brief Checks if a node is selected.
         * @param nodeId Node ID to check
         * @return True if node is selected
         */
        [[nodiscard]] bool IsNodeSelected(Engine::NodeId nodeId) const;

        /**
         * @brief Checks if any nodes are selected.
         * @return True if selection is not empty
         */
        [[nodiscard]] bool HasSelection() const;

        /**
         * @brief Returns the number of selected nodes.
         * @return Number of selected nodes
         */
        [[nodiscard]] size_t GetSelectionCount() const;

        /**
         * @brief Returns all selected node IDs.
         * @return Reference to set of selected node IDs
         */
        [[nodiscard]] const std::unordered_set<Engine::NodeId> &GetSelectedNodes() const;

        /**
         * @brief Selects a single node (clears previous selection).
         * @param nodeId Node ID to select
         */
        void SelectNode(Engine::NodeId nodeId);

        /**
         * @brief Toggles node selection (for Shift+click).
         * @param nodeId Node ID to toggle
         */
        void ToggleNodeSelection(Engine::NodeId nodeId);

        /**
         * @brief Adds node to selection without clearing.
         * @param nodeId Node ID to add
         */
        void AddToSelection(Engine::NodeId nodeId);

        /**
         * @brief Removes node from selection.
         * @param nodeId Node ID to remove
         */
        void RemoveFromSelection(Engine::NodeId nodeId);

        /**
         * @brief Clears all selection.
         */
        void ClearSelection();

        /**
         * @brief Returns the primary selected node (for backward compatibility).
         * @return Node ID or kInvalidNodeId
         */
        [[nodiscard]] Engine::NodeId GetPrimarySelection() const;

        /**
         * @brief Starts box selection.
         * @param startPos Starting position in screen space
         */
        void StartBoxSelection(const ImVec2 &startPos);

        /**
         * @brief Updates box selection end position.
         * @param endPos End position in screen space
         */
        void UpdateBoxSelection(const ImVec2 &endPos);

        /**
         * @brief Ends box selection.
         */
        void EndBoxSelection();

        /**
         * @brief Checks if box selection is active.
         * @return True if box selecting
         */
        [[nodiscard]] bool IsBoxSelecting() const;

        /**
         * @brief Gets box selection rectangle bounds.
         * @return Pair of min and max positions
         */
        [[nodiscard]] std::pair<ImVec2, ImVec2> GetBoxSelectionBounds() const;

        /**
         * @brief Starts dragging selected nodes.
         * @param mousePos Current mouse position
         * @param nodePositions Map of node positions
         */
        void StartDrag(const ImVec2 &mousePos, const std::unordered_map<Engine::NodeId, ImVec2> &nodePositions);

        /**
         * @brief Stops dragging.
         */
        void StopDrag();

        /**
         * @brief Checks if nodes are being dragged.
         * @return True if dragging
         */
        [[nodiscard]] bool IsDragging() const;

        /**
         * @brief Gets drag offset for a node.
         * @param nodeId Node ID
         * @return Drag offset, or zero if not found
         */
        [[nodiscard]] ImVec2 GetDragOffset(Engine::NodeId nodeId) const;

        /**
         * @brief Gets all drag offsets.
         * @return Reference to drag offsets map
         */
        [[nodiscard]] const std::unordered_map<Engine::NodeId, ImVec2> &GetDragOffsets() const;

    private:
        // Selection state
        std::unordered_set<Engine::NodeId> selectedNodes;
        Engine::NodeId primarySelection = -1; ///< Primary selected node (for backward compatibility)

        // Box selection state
        bool isBoxSelecting = false;
        ImVec2 boxSelectStart{ 0.0f, 0.0f };
        ImVec2 boxSelectEnd{ 0.0f, 0.0f };

        // Drag state
        bool isDragging = false;
        std::unordered_map<Engine::NodeId, ImVec2> dragOffsets;
    };
} // namespace VisionCraft
