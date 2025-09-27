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
     * @brief Node editor layer.
     *
     * NodeEditorLayer implements the complete node editing experience combining
     * canvas management (pan/zoom/grid) with node rendering and interactions.
     * Similar to UE Blueprints, everything is contained in a single unified interface.
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
         * @brief Handles canvas input events.
         * @param event The event to handle (mouse, keyboard, etc.)
         *
         * Processes mouse wheel events for zooming and mouse drag events for panning.
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the canvas state.
         * @param deltaTime Time elapsed since the last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders the canvas and grid.
         *
         * Draws the infinite grid background and provides the drawing surface
         * for the node editor. Uses ImGui's draw list for custom rendering.
         */
        void OnRender() override;

    private:
        /**
         * @brief Renders all nodes in the editor.
         */
        void RenderNodes();

        /**
         * @brief Renders a single node.
         * @param node Pointer to the node to render
         * @param nodePos Position of the node
         */
        void RenderNode(Engine::Node *node, const NodePosition &nodePos);

        /**
         * @brief Tests if a mouse position hits a node.
         * @param mousePos Mouse position in screen coordinates
         * @param nodePos Node position in world coordinates
         * @param nodeSize Node size in pixels
         * @return True if mouse hits the node
         */
        [[nodiscard]] bool
            IsMouseOverNode(const ImVec2 &mousePos, const NodePosition &nodePos, const ImVec2 &nodeSize) const;

        /**
         * @brief Handles mouse interactions for node selection and dragging.
         */
        void HandleMouseInteractions();

        /**
         * @brief Detects which pin is currently under the mouse cursor.
         */
        void DetectHoveredPin();

        /**
         * @brief Gets the interaction state for a specific pin.
         * @param nodeId ID of the node containing the pin
         * @param pinName Name of the pin
         * @return PinInteractionState containing hover and active flags
         */
        [[nodiscard]] PinInteractionState GetPinInteractionState(Engine::NodeId nodeId,
            const std::string &pinName) const;


        /**
         * @brief Render a pin with its label text.
         * @param pin The pin to render
         * @param pinPos Screen position of the pin
         * @param labelPos Screen position of the label
         * @param pinRadius Radius of the pin circle
         * @param zoomLevel Current zoom level
         * @param state Pin interaction state (hover/active)
         */
        void RenderPinWithLabel(const NodePin &pin,
            const ImVec2 &pinPos,
            const ImVec2 &labelPos,
            float pinRadius,
            float zoomLevel,
            const PinInteractionState &state) const;

        /**
         * @brief Renders the context menu for creating nodes.
         */
        void RenderContextMenu();

        /**
         * @brief Creates a new node of the specified type at the given position.
         * @param nodeType Type of node to create
         * @param position Position to place the node
         */
        void CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position);


        /**
         * @brief Gets the color for a specific data type.
         * @param dataType The data type
         * @return ImU32 color value
         */
        [[nodiscard]] ImU32 GetDataTypeColor(PinDataType dataType) const;


        /**
         * @brief Finds the topmost node at the given mouse position.
         * @param mousePos Mouse position in screen coordinates
         * @return Node ID if found, or -1 if no node at position
         */
        [[nodiscard]] Engine::NodeId FindNodeAtPosition(const ImVec2 &mousePos) const;


        // Core components
        CanvasController canvas;                                        ///< Canvas management component
        ConnectionManager connectionManager;                            ///< Connection management component
        Engine::NodeEditor nodeEditor;                                  ///< Backend node editor
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