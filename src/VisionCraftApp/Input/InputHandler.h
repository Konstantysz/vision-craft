#pragma once

#include "Canvas/CanvasController.h"
#include "Editor/NodeEditorTypes.h"
#include "Input/SelectionManager.h"
#include "UI/ContextMenuRenderer.h"

#include <functional>
#include <optional>
#include <unordered_map>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Callback to find node at position.
     */
    using FindNodeCallback = std::function<Engine::NodeId(const ImVec2 &)>;

    /**
     * @brief Callback to find connection at position.
     */
    using FindConnectionCallback = std::function<std::optional<NodeConnection>(const ImVec2 &)>;

    /**
     * @brief Callback to update box selection.
     */
    using UpdateBoxSelectionCallback = std::function<void()>;

    enum class InputActionType
    {
        None,
        DeleteNodes,
        DeleteConnection,
        OpenContextMenu,
        UpdateNodePositions,
        UpdateHoveredConnection,
        CopyNodes,
        CutNodes,
        PasteNodes
    };

    /**
     * @brief Input action results.
     */
    struct InputAction
    {
        InputActionType type = InputActionType::None;                   ///< Type of the input action
        std::vector<Engine::NodeId> nodeIds;                            ///< For DeleteNodes
        std::optional<NodeConnection> connection;                       ///< For DeleteConnection
        ImVec2 contextMenuPos{ 0.0f, 0.0f };                            ///< For OpenContextMenu
        std::unordered_map<Engine::NodeId, NodePosition> nodePositions; ///< For UpdateNodePositions
        std::optional<NodeConnection> hoveredConnection;                ///< For UpdateHoveredConnection
        ImVec2 pastePosition{ 0.0f, 0.0f };                             ///< For PasteNodes
    };

    /**
     * @brief Handles mouse and keyboard input for node editor.
     *
     * Separates input handling logic from layer rendering.
     * Coordinates with SelectionManager and ContextMenuRenderer.
     */
    class InputHandler
    {
    public:
        /**
         * @brief Constructor.
         * @param selectionManager Reference to selection manager
         * @param contextMenuRenderer Reference to context menu renderer
         * @param canvas Reference to canvas controller
         */
        InputHandler(SelectionManager &selectionManager,
            ContextMenuRenderer &contextMenuRenderer,
            CanvasController &canvas);

        /**
         * @brief Processes mouse and keyboard input.
         * @param nodePositions Current node positions
         * @param hoveredPin Currently hovered pin
         * @param findNode Callback to find node at position
         * @param findConnection Callback to find connection at position
         * @param updateBoxSelection Callback to update box selection
         * @return Vector of input actions to process
         */
        [[nodiscard]] std::vector<InputAction> ProcessInput(
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const PinId &hoveredPin,
            const FindNodeCallback &findNode,
            const FindConnectionCallback &findConnection,
            const UpdateBoxSelectionCallback &updateBoxSelection);

        /**
         * @brief Gets the context menu position from last right-click.
         * @return Context menu position
         */
        [[nodiscard]] ImVec2 GetContextMenuPos() const
        {
            return contextMenuPos;
        }

    private:
        /**
         * @brief Handles Delete key press.
         * @return Delete action if key pressed
         */
        [[nodiscard]] std::optional<InputAction> HandleDeleteKey();

        /**
         * @brief Handles right mouse button click.
         * @param mousePos Mouse position
         * @param findNode Callback to find node at position
         * @return Vector of actions (delete connection, open context menu)
         */
        [[nodiscard]] std::vector<InputAction> HandleRightClick(const ImVec2 &mousePos,
            const FindNodeCallback &findNode);

        /**
         * @brief Handles left mouse button click.
         * @param mousePos Mouse position
         * @param nodePositions Current node positions
         * @param findNode Callback to find node at position
         */
        void HandleLeftClick(const ImVec2 &mousePos,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const FindNodeCallback &findNode);

        /**
         * @brief Handles mouse drag for box selection and node dragging.
         * @param mousePos Mouse position
         * @param nodePositions Current node positions
         * @param updateBoxSelection Callback to update box selection
         * @return Vector of actions (update node positions)
         */
        [[nodiscard]] std::vector<InputAction> HandleMouseDrag(const ImVec2 &mousePos,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const UpdateBoxSelectionCallback &updateBoxSelection);

        /**
         * @brief Handles mouse button release.
         */
        void HandleMouseRelease();

        /**
         * @brief Updates hovered connection.
         * @param mousePos Mouse position
         * @param hoveredPin Currently hovered pin
         * @param findConnection Callback to find connection at position
         * @return Action to update hovered connection
         */
        [[nodiscard]] InputAction UpdateHoveredConnection(const ImVec2 &mousePos,
            const PinId &hoveredPin,
            const FindConnectionCallback &findConnection);

        SelectionManager &selectionManager;
        ContextMenuRenderer &contextMenuRenderer;
        CanvasController &canvas;

        ImVec2 contextMenuPos{ 0.0f, 0.0f };
        std::optional<NodeConnection> hoveredConnection;
    };
} // namespace VisionCraft
