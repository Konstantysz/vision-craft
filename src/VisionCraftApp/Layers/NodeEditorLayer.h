#pragma once

#include "Layer.h"
#include "NodeEditor.h"

#include <memory>
#include <unordered_map>

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
    };
} // namespace VisionCraft