#pragma once

#include "CanvasController.h"
#include "ConnectionManager.h"
#include "Layer.h"
#include "NodeEditor.h"
#include "NodeEditorTypes.h"
#include "NodeRenderer.h"

#include <memory>
#include <unordered_map>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Node editor layer with canvas, rendering, and interactions.
     */
    class NodeEditorLayer : public Core::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        NodeEditorLayer();

        /**
         * @brief Virtual destructor.
         */
        virtual ~NodeEditorLayer() = default;

        /**
         * @brief Handles input events.
         * @param event Event to handle
         */
        void OnEvent(Core::Event &event) override;

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
         * @param node Node to render
         * @param nodePos Node position
         */
        void RenderNode(Engine::Node *node, const NodePosition &nodePos);

        /**
         * @brief Checks if mouse is over node.
         * @param mousePos Mouse position
         * @param nodePos Node position
         * @param nodeSize Node size
         * @return True if over node
         */
        [[nodiscard]] bool
            IsMouseOverNode(const ImVec2 &mousePos, const NodePosition &nodePos, const ImVec2 &nodeSize) const;

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
         * @param nodeId Node ID
         * @param pinName Pin name
         * @return Pin interaction state
         */
        [[nodiscard]] PinInteractionState GetPinInteractionState(Engine::NodeId nodeId,
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
        void RenderPinWithLabel(const NodePin &pin,
            const ImVec2 &pinPos,
            const ImVec2 &labelPos,
            float pinRadius,
            float zoomLevel,
            const PinInteractionState &state) const;

        /**
         * @brief Renders context menu.
         */
        void RenderContextMenu();

        /**
         * @brief Creates node at position.
         * @param nodeType Node type
         * @param position Position to place node
         */
        void CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position);

        /**
         * @brief Returns data type color.
         * @param dataType Data type
         * @return Color
         */
        [[nodiscard]] ImU32 GetDataTypeColor(PinDataType dataType) const;

        /**
         * @brief Finds node at position.
         * @param mousePos Mouse position
         * @return Node ID or -1
         */
        [[nodiscard]] Engine::NodeId FindNodeAtPosition(const ImVec2 &mousePos) const;

        /**
         * @brief Returns shared node editor.
         * @return Node editor
         */
        [[nodiscard]] Engine::NodeEditor &GetNodeEditor();

        /**
         * @brief Returns shared node editor.
         * @return Node editor
         */
        [[nodiscard]] const Engine::NodeEditor &GetNodeEditor() const;

        // Core components
        CanvasController canvas;                                        ///< Canvas management component
        ConnectionManager connectionManager;                            ///< Connection management component
        NodeRenderer nodeRenderer;                                      ///< Node rendering component
        std::unordered_map<Engine::NodeId, NodePosition> nodePositions; ///< Visual positions of nodes
        Engine::NodeId nextNodeId = 1;                                  ///< Next available node ID

        // Selection and dragging state
        Engine::NodeId selectedNodeId = Constants::Special::kInvalidNodeId; ///< Currently selected node ID
        bool isDragging = false;                                            ///< Whether a node is being dragged
        ImVec2 dragOffset = ImVec2(0.0f, 0.0f); ///< Mouse offset from node position during drag

        // Context menu state
        bool showContextMenu = false;               ///< Whether to show the context menu
        ImVec2 contextMenuPos = ImVec2(0.0f, 0.0f); ///< Position where context menu was opened

        // Pin interaction state
        PinId hoveredPin = { Constants::Special::kInvalidNodeId, "" }; ///< Currently hovered pin
    };
} // namespace VisionCraft