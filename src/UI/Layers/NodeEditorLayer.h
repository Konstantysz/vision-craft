#pragma once

#include "Editor/Commands/CommandHistory.h"
#include "Editor/State/ClipboardManager.h"
#include "Editor/State/SelectionManager.h"
#include "UI/Canvas/CanvasController.h"
#include "UI/Canvas/ConnectionManager.h"
#include "UI/Canvas/InputHandler.h"
#include "UI/Rendering/NodeRenderer.h"
#include "UI/Widgets/ContextMenuRenderer.h"
#include "UI/Widgets/FileDialogManager.h"
#include "UI/Widgets/NodeEditorTypes.h"
#include "Layer.h"
#include "Nodes/Core/NodeEditor.h"
#include "Nodes/Factory/NodeFactory.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <imgui.h>

namespace VisionCraft::UI::Layers
{
    /**
     * @brief Nodes::Node editor layer with canvas, rendering, and interactions.
     */
    class NodeEditorLayer : public Kappa::Layer
    {
    public:
        /**
         * @brief Constructor that initializes the node editor layer.
         * @param nodeEditor Reference to the shared node editor instance
         */
        explicit NodeEditorLayer(Nodes::NodeEditor &nodeEditor);

        /**
         * @brief Virtual destructor.
         */
        virtual ~NodeEditorLayer() = default;

        /**
         * @brief Handles input events.
         * @param event Event to handle
         */
        void OnEvent(Kappa::Event &event) override;

        /**
         * @brief Updates editor state.
         * @param deltaTime Time since last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders canvas and nodes.
         */
        void OnRender() override;

    private:
        /**
         * @brief Renders all nodes.
         */
        void RenderNodes();

        /**
         * @brief Renders single node.
         * @param node Nodes::Node to render
         * @param nodePos Nodes::Node position
         */
        void RenderNode(Nodes::Node *node, const Widgets::NodePosition &nodePos);

        /**
         * @brief Checks if mouse is over node.
         * @param mousePos Mouse position
         * @param nodePos Nodes::Node position
         * @param nodeSize Nodes::Node size
         * @return True if over node
         */
        [[nodiscard]] bool
            IsMouseOverNode(const ImVec2 &mousePos, const Widgets::NodePosition &nodePos, const ImVec2 &nodeSize) const;

        /**
         * @brief Handles mouse interactions.
         */
        void HandleMouseInteractions();

        /**
         * @brief Detects hovered pin.
         */
        void DetectHoveredPin();

        /**
         * @brief Returns pin interaction state.
         * @param nodeId Nodes::Node ID
         * @param pinName Pin name
         * @return Pin interaction state
         */
        [[nodiscard]] Rendering::PinInteractionState GetPinInteractionState(Nodes::NodeId nodeId,
            const std::string &pinName) const;

        /**
         * @brief Renders pin with label.
         * @param pin Pin to render
         * @param pinPos Pin position
         * @param labelPos Label position
         * @param pinRadius Pin radius
         * @param zoomLevel Zoom level
         * @param state Interaction state
         */
        void RenderPinWithLabel(const Widgets::NodePin &pin,
            const ImVec2 &pinPos,
            const ImVec2 &labelPos,
            float pinRadius,
            float zoomLevel,
            const Rendering::PinInteractionState &state) const;

        /**
         * @brief Renders context menu.
         */
        void RenderContextMenu();

        /**
         * @brief Creates node at position.
         * @param nodeType Nodes::Node type
         * @param position Position to place node
         */
        void CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position);

        /**
         * @brief Deletes a node.
         * @param nodeId Nodes::Node ID to delete
         */
        void DeleteNode(Nodes::NodeId nodeId);

        /**
         * @brief Returns data type color.
         * @param dataType Data type
         * @return Color
         */
        [[nodiscard]] ImU32 GetDataTypeColor(Widgets::PinDataType dataType) const;

        /**
         * @brief Finds node at position.
         * @param mousePos Mouse position
         * @return Nodes::Node ID or -1
         */
        [[nodiscard]] Nodes::NodeId FindNodeAtPosition(const ImVec2 &mousePos) const;

        /**
         * @brief Renders box selection rectangle.
         */
        void RenderBoxSelection();

        /**
         * @brief Updates box selection based on current mouse position.
         */
        void UpdateBoxSelection();

        /**
         * @brief Checks if node is selected.
         * @param nodeId Nodes::Node ID to check
         * @return True if selected
         */
        [[nodiscard]] bool IsNodeSelected(Nodes::NodeId nodeId) const;


        /**
         * @brief Handles save graph event.
         */
        void HandleSaveGraph();

        /**
         * @brief Handles load graph event (opens file dialog).
         */
        void HandleLoadGraph();

        /**
         * @brief Handles load graph from specific file.
         * @param filePath Path to the graph file
         */
        void HandleLoadGraphFromFile(const std::string &filePath);

        /**
         * @brief Handles new graph event.
         */
        void HandleNewGraph();

        /**
         * @brief Renders save file dialog.
         */
        void RenderSaveDialog();

        /**
         * @brief Renders load file dialog.
         */
        void RenderLoadDialog();

        /**
         * @brief Converts node class type to factory registration key.
         * @param nodeType Nodes::Node class type (e.g., "GrayscaleNode")
         * @return Factory key (e.g., "Grayscale")
         */
        [[nodiscard]] std::string NodeTypeToFactoryKey(const std::string &nodeType) const;

        /**
         * @brief Allocates next available node ID with overflow checking.
         * @return Next available node ID, or 0 on overflow
         */
        [[nodiscard]] Nodes::NodeId AllocateNodeId();

        // Core components
        Nodes::NodeEditor &nodeEditor;                    ///< Reference to the shared node editor instance
        Canvas::CanvasController canvas;                  ///< Canvas management component
        Canvas::ConnectionManager connectionManager;      ///< Connection management component
        Rendering::NodeRenderer nodeRenderer;             ///< Nodes::Node rendering component
        Editor::State::SelectionManager selectionManager; ///< Manages node selection and dragging
        Widgets::ContextMenuRenderer contextMenuRenderer; ///< Renders context menu
        Widgets::FileDialogManager fileDialogManager;     ///< Manages file dialogs
        Canvas::InputHandler inputHandler;                ///< Handles input processing
        Editor::State::ClipboardManager clipboardManager; ///< Manages copy/cut/paste operations
        Editor::Commands::CommandHistory commandHistory;  ///< Manages undo/redo history
        std::unordered_map<Nodes::NodeId, Widgets::NodePosition> nodePositions; ///< Visual positions of nodes
        Nodes::NodeId nextNodeId = 1;                                           ///< Next available node ID

        // Pin interaction state
        Widgets::PinId hoveredPin = { Constants::Special::kInvalidNodeId, "" }; ///< Currently hovered pin

        // Connection interaction state
        std::optional<Widgets::NodeConnection> hoveredConnection = std::nullopt; ///< Currently hovered connection

        // File management state
        std::string currentFilePath; ///< Current file path
    };
} // namespace VisionCraft::UI::Layers
