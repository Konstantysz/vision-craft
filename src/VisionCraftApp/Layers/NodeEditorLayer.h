#pragma once

#include "Layer.h"
#include "NodeEditor.h"

#include <memory>
#include <unordered_map>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Structure representing the visual position of a node in the editor.
     */
    struct NodePosition
    {
        float x = 0.0f;
        float y = 0.0f;
    };

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
        NodeEditorLayer() = default;

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
        float zoomLevel = 1.0f; ///< Current zoom level (1.0 = 100%)
        float panX = 0.0f;      ///< Horizontal pan offset in pixels
        float panY = 0.0f;      ///< Vertical pan offset in pixels
        float gridSize = 20.0f; ///< Size of grid cells in pixels
        bool showGrid = true;   ///< Whether to display the grid background

        Engine::NodeEditor nodeEditor;                                  ///< Backend node editor
        std::unordered_map<Engine::NodeId, NodePosition> nodePositions; ///< Visual positions of nodes
        Engine::NodeId nextNodeId = 1;                                  ///< Next available node ID

        // Selection and dragging state
        Engine::NodeId selectedNodeId = -1;                             ///< Currently selected node ID (-1 = none)
        bool isDragging = false;                                         ///< Whether a node is being dragged
        ImVec2 dragOffset = ImVec2(0.0f, 0.0f);                        ///< Mouse offset from node position during drag

        // Context menu state
        bool showContextMenu = false;                                    ///< Whether to show the context menu
        ImVec2 contextMenuPos = ImVec2(0.0f, 0.0f);                    ///< Position where context menu was opened
        ImVec2 currentCanvasPos = ImVec2(0.0f, 0.0f);                  ///< Current canvas position for coordinate calculations

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
        bool IsMouseOverNode(const ImVec2& mousePos, const NodePosition& nodePos, const ImVec2& nodeSize) const;

        /**
         * @brief Handles mouse interactions for node selection and dragging.
         */
        void HandleMouseInteractions();

        /**
         * @brief Renders the context menu for creating nodes.
         */
        void RenderContextMenu();

        /**
         * @brief Creates a new node of the specified type at the given position.
         * @param nodeType Type of node to create
         * @param position Position to place the node
         */
        void CreateNodeAtPosition(const std::string& nodeType, const ImVec2& position);
    };
} // namespace VisionCraft