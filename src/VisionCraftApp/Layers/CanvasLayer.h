#pragma once

#include "Layer.h"

namespace VisionCraft
{
    /**
     * @brief Layer that provides an infinite canvas with grid background and pan/zoom functionality.
     *
     * CanvasLayer implements the main workspace where users can view and interact with
     * the node graph. It provides an infinite scrollable canvas with a grid background,
     * zoom functionality, and panning capabilities. This layer serves as the foundation
     * for the node editor's visual workspace.
     */
    class CanvasLayer : public Core::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        CanvasLayer() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~CanvasLayer() = default;

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
        float zoomLevel = 1.0f;    ///< Current zoom level (1.0 = 100%)
        float panX = 0.0f;         ///< Horizontal pan offset in pixels
        float panY = 0.0f;         ///< Vertical pan offset in pixels
        float gridSize = 20.0f;    ///< Size of grid cells in pixels
        bool showGrid = true;      ///< Whether to display the grid background
    };
} // namespace VisionCraft