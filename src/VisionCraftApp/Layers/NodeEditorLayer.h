#pragma once

#include "Layer.h"

#include <memory>
#include <vector>

#include "Node.h"

namespace VisionCraft
{
    /**
     * @brief Layer that manages the node editor interface and interactions.
     *
     * NodeEditorLayer handles the creation, manipulation, and rendering of nodes
     * within the canvas. It manages node selection, dragging, connection creation,
     * and other node-specific interactions. This layer works in conjunction with
     * the CanvasLayer to provide a complete node editing experience.
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
         * @brief Handles node editor events.
         * @param event The event to handle
         *
         * Processes mouse events for node selection, dragging, and connection creation.
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the node editor state.
         * @param deltaTime Time elapsed since the last update
         *
         * Updates node positions during dragging and handles animation states.
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders all nodes and their connections.
         *
         * Draws all nodes in the editor, their input/output pins, connections
         * between nodes, and selection indicators.
         */
        void OnRender() override;

    private:
        std::vector<std::unique_ptr<Engine::Node>> nodes; ///< Collection of nodes in the editor
        Engine::Node *selectedNode = nullptr;             ///< Currently selected node
        bool isDragging = false;                          ///< Whether a node is being dragged
        float dragOffsetX = 0.0f;                         ///< X offset from mouse to node center during drag
        float dragOffsetY = 0.0f;                         ///< Y offset from mouse to node center during drag
    };
} // namespace VisionCraft