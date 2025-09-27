#pragma once

#include "Event.h"
#include "NodeEditorConstants.h"

#include <imgui.h>

/**
 * @file CanvasController.h
 * @brief Canvas management component for the node editor.
 *
 * This class handles all canvas-related operations including pan, zoom, grid rendering,
 * and coordinate transformations. It follows the Single Responsibility Principle by
 * separating canvas logic from the main node editor.
 */

namespace VisionCraft
{
    /**
     * @brief Component responsible for canvas management in the node editor.
     *
     * The CanvasController handles:
     * - Pan and zoom operations
     * - Grid background rendering
     * - Coordinate transformations between screen and world space
     * - Canvas input handling (mouse wheel, middle mouse drag)
     * - Canvas bounds and limits
     *
     * This component follows the Single Responsibility Principle and can be
     * easily tested in isolation or replaced with different canvas behaviors.
     */
    class CanvasController
    {
    public:
        /**
         * @brief Default constructor with standard canvas settings.
         */
        CanvasController();

        /**
         * @brief Virtual destructor for potential inheritance.
         */
        virtual ~CanvasController() = default;

        /**
         * @brief Handles canvas-related input events.
         * @param event The input event to process
         * @param mousePos Current mouse position in screen coordinates
         * @param isWindowHovered Whether the window is currently hovered
         *
         * Processes mouse wheel events for zooming and mouse drag events for panning.
         * Only handles events when the window is hovered and not conflicting with other UI.
         */
        void HandleInput(Core::Event &event, const ImVec2 &mousePos, bool isWindowHovered);

        /**
         * @brief Handles ImGui-specific canvas input during the render loop.
         * @param io ImGui IO object containing input state
         * @param isWindowHovered Whether the window is currently hovered
         *
         * This handles immediate-mode input that needs to be processed during rendering.
         */
        void HandleImGuiInput(const ImGuiIO &io, bool isWindowHovered);

        /**
         * @brief Begins the canvas rendering context.
         * @param drawList ImGui draw list for rendering
         * @param canvasPos Screen position of the canvas area
         * @param canvasSize Size of the canvas area
         *
         * Sets up the canvas for rendering and draws the grid background.
         * Must be called before any world-space rendering.
         */
        void BeginCanvas(ImDrawList *drawList, const ImVec2 &canvasPos, const ImVec2 &canvasSize);

        /**
         * @brief Ends the canvas rendering context.
         *
         * Performs any cleanup needed after canvas rendering is complete.
         */
        void EndCanvas();

        /**
         * @brief Converts screen coordinates to world coordinates.
         * @param screenPos Position in screen coordinates
         * @return Position in world coordinates
         */
        [[nodiscard]] ImVec2 ScreenToWorld(const ImVec2 &screenPos) const;

        /**
         * @brief Converts world coordinates to screen coordinates.
         * @param worldPos Position in world coordinates
         * @return Position in screen coordinates
         */
        [[nodiscard]] ImVec2 WorldToScreen(const ImVec2 &worldPos) const;

        /**
         * @brief Gets the current canvas position in screen coordinates.
         * @return Canvas position
         */
        [[nodiscard]] ImVec2 GetCanvasPosition() const
        {
            return currentCanvasPos;
        }

        /**
         * @brief Gets the current canvas size.
         * @return Canvas size
         */
        [[nodiscard]] ImVec2 GetCanvasSize() const
        {
            return currentCanvasSize;
        }

        /**
         * @brief Gets the current zoom level.
         * @return Current zoom level (1.0 = 100%)
         */
        [[nodiscard]] float GetZoomLevel() const
        {
            return zoomLevel;
        }

        /**
         * @brief Sets the zoom level with bounds checking.
         * @param newZoom Target zoom level
         *
         * Automatically clamps the zoom level to valid bounds.
         */
        void SetZoomLevel(float newZoom);

        /**
         * @brief Applies a zoom delta (e.g., from mouse wheel).
         * @param delta Zoom change amount
         */
        void ApplyZoomDelta(float delta);

        /**
         * @brief Zooms to fit content in the canvas.
         * @param contentBounds Bounding box of content to fit
         * @param padding Optional padding around content
         */
        void ZoomToFit(const ImVec2 &contentMin, const ImVec2 &contentMax, float padding = 50.0f);

        /**
         * @brief Gets the current pan offset.
         * @return Current pan offset in pixels
         */
        [[nodiscard]] ImVec2 GetPanOffset() const
        {
            return ImVec2(panX, panY);
        }

        /**
         * @brief Sets the pan offset.
         * @param offset New pan offset in pixels
         */
        void SetPanOffset(const ImVec2 &offset);

        /**
         * @brief Applies a pan delta (e.g., from mouse drag).
         * @param delta Pan change amount in pixels
         */
        void ApplyPanDelta(const ImVec2 &delta);

        /**
         * @brief Centers the view on a specific world position.
         * @param worldPos Position to center on in world coordinates
         */
        void CenterOn(const ImVec2 &worldPos);

        /**
         * @brief Gets whether the grid is currently visible.
         * @return True if grid is shown
         */
        [[nodiscard]] bool IsGridVisible() const
        {
            return showGrid;
        }

        /**
         * @brief Sets grid visibility.
         * @param visible True to show grid, false to hide
         */
        void SetGridVisible(bool visible)
        {
            showGrid = visible;
        }

        /**
         * @brief Toggles grid visibility.
         */
        void ToggleGrid()
        {
            showGrid = !showGrid;
        }

        /**
         * @brief Gets the current grid size.
         * @return Grid cell size in pixels
         */
        [[nodiscard]] float GetGridSize() const
        {
            return gridSize;
        }

        /**
         * @brief Sets the grid size.
         * @param size New grid cell size in pixels
         */
        void SetGridSize(float size)
        {
            gridSize = size;
        }

        /**
         * @brief Resets the canvas to default view (zoom 1.0, pan 0,0).
         */
        void ResetView();

        /**
         * @brief Checks if a world-space rectangle is visible in the current view.
         * @param worldMin Top-left corner of rectangle in world coordinates
         * @param worldMax Bottom-right corner of rectangle in world coordinates
         * @return True if rectangle is at least partially visible
         */
        bool IsRectangleVisible(const ImVec2 &worldMin, const ImVec2 &worldMax) const;

        /**
         * @brief Gets the visible world-space bounds of the current view.
         * @param outMin Output parameter for top-left visible world position
         * @param outMax Output parameter for bottom-right visible world position
         */
        void GetVisibleWorldBounds(ImVec2 &outMin, ImVec2 &outMax) const;

    private:
        /**
         * @brief Renders the grid background.
         *
         * Draws an infinite grid pattern based on current zoom and pan state.
         * Only called internally during BeginCanvas().
         */
        void RenderGrid();

        /**
         * @brief Clamps zoom level to valid bounds.
         * @param zoom Input zoom level
         * @return Clamped zoom level
         */
        float ClampZoom(float zoom) const;

        /**
         * @brief Ensures canvas size meets minimum requirements.
         * @param size Input canvas size
         * @return Size with minimum bounds applied
         */
        ImVec2 EnsureMinimumCanvasSize(const ImVec2 &size) const;

        // Canvas transform state
        float zoomLevel = Constants::Zoom::kDefault; ///< Current zoom level (1.0 = 100%)
        float panX = 0.0f;                           ///< Horizontal pan offset in pixels
        float panY = 0.0f;                           ///< Vertical pan offset in pixels

        // Grid settings
        float gridSize = Constants::Canvas::kGridSize; ///< Size of grid cells in pixels
        bool showGrid = true;                          ///< Whether to display the grid background

        // Current render context
        ImDrawList *currentDrawList = nullptr;   ///< Current ImGui draw list
        ImVec2 currentCanvasPos = ImVec2(0, 0);  ///< Current canvas position in screen coordinates
        ImVec2 currentCanvasSize = ImVec2(0, 0); ///< Current canvas size
    };

} // namespace VisionCraft