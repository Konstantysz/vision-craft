#pragma once

#include "Editor/State/SelectionManager.h"
#include "UI/Canvas/CanvasController.h"
#include "UI/Widgets/ContextMenuRenderer.h"
#include "UI/Widgets/NodeEditorTypes.h"

#include <functional>
#include <optional>
#include <unordered_map>

#include <imgui.h>

namespace VisionCraft::UI::Canvas
{
    /**
     * @brief Callback to find node at position.
     */
    using FindNodeCallback = std::function<Nodes::NodeId(const ImVec2 &)>;

    /**
     * @brief Callback to find connection at position.
     */
    using FindConnectionCallback = std::function<std::optional<UI::Widgets::NodeConnection>(const ImVec2 &)>;

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
        PasteNodes,
        Undo,
        Redo,
        FinishNodeMove
    };

    /**
     * @brief Input action results.
     */
    struct InputAction
    {
        InputActionType type = InputActionType::None;          ///< Type of the input action
        std::vector<Nodes::NodeId> nodeIds;                    ///< For DeleteNodes
        std::optional<UI::Widgets::NodeConnection> connection; ///< For DeleteConnection
        ImVec2 contextMenuPos{ 0.0f, 0.0f };                   ///< For OpenContextMenu
        std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition>
            nodePositions;                                            ///< For UpdateUI::Widgets::NodePositions
        std::optional<UI::Widgets::NodeConnection> hoveredConnection; ///< For UpdateHoveredConnection
        ImVec2 pastePosition{ 0.0f, 0.0f };                           ///< For PasteNodes
        std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition>
            oldNodePositions; ///< For FinishNodeMove (old positions)
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
        InputHandler(Editor::State::SelectionManager &selectionManager,
            UI::Widgets::ContextMenuRenderer &contextMenuRenderer,
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
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const UI::Widgets::PinId &hoveredPin,
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
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const FindNodeCallback &findNode);

        /**
         * @brief Handles mouse drag for box selection and node dragging.
         * @param mousePos Mouse position
         * @param nodePositions Current node positions
         * @param updateBoxSelection Callback to update box selection
         * @return Vector of actions (update node positions)
         */
        [[nodiscard]] std::vector<InputAction> HandleMouseDrag(const ImVec2 &mousePos,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const UpdateBoxSelectionCallback &updateBoxSelection);

        /**
         * @brief Handles mouse button release.
         * @param nodePositions Current node positions
         * @return Vector of actions (finish node move)
         */
        [[nodiscard]] std::vector<InputAction> HandleMouseRelease(
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions);

        /**
         * @brief Updates hovered connection.
         * @param mousePos Mouse position
         * @param hoveredPin Currently hovered pin
         * @param findConnection Callback to find connection at position
         * @return Action to update hovered connection
         */
        [[nodiscard]] InputAction UpdateHoveredConnection(const ImVec2 &mousePos,
            const UI::Widgets::PinId &hoveredPin,
            const FindConnectionCallback &findConnection);

        Editor::State::SelectionManager &selectionManager;
        UI::Widgets::ContextMenuRenderer &contextMenuRenderer;
        CanvasController &canvas;

        ImVec2 contextMenuPos{ 0.0f, 0.0f };
        std::optional<UI::Widgets::NodeConnection> hoveredConnection;
        std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition>
            dragStartPositions; ///< Nodes::Node positions when drag started
    };
} // namespace VisionCraft::UI::Canvas
