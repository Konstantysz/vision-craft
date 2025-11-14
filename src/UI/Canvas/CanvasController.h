#pragma once

#include "UI/Widgets/NodeEditorConstants.h"
#include "Event.h"

#include <imgui.h>

namespace VisionCraft::UI::Canvas
{
    /**
     * @brief Canvas management for pan, zoom, grid, and coordinate transformations.
     */
    class CanvasController
    {
    public:
        /**
         * @brief Default constructor with standard canvas settings.
         */
        CanvasController();

        /**
         * @brief Handles canvas input events.
         * @param event Event to process
         * @param mousePos Mouse position in screen coordinates
         * @param isWindowHovered Whether window is hovered
         */
        void HandleInput(Kappa::Event &event, const ImVec2 &mousePos, bool isWindowHovered);

        /**
         * @brief Handles ImGui-specific canvas input.
         * @param io ImGui IO object
         * @param isWindowHovered Whether window is hovered
         */
        void HandleImGuiInput(const ImGuiIO &io, bool isWindowHovered);

        /**
         * @brief Begins canvas rendering context.
         * @param drawList ImGui draw list
         * @param canvasPos Canvas position in screen coordinates
         * @param canvasSize Canvas size
         */
        void BeginCanvas(ImDrawList *drawList, const ImVec2 &canvasPos, const ImVec2 &canvasSize);

        /**
         * @brief Ends canvas rendering context.
         */
        void EndCanvas();

        /**
         * @brief Converts screen to world coordinates.
         * @param screenPos Screen position
         * @return World position
         */
        [[nodiscard]] ImVec2 ScreenToWorld(const ImVec2 &screenPos) const;

        /**
         * @brief Converts world to screen coordinates.
         * @param worldPos World position
         * @return Screen position
         */
        [[nodiscard]] ImVec2 WorldToScreen(const ImVec2 &worldPos) const;

        /**
         * @brief Returns canvas position.
         * @return Canvas position
         */
        [[nodiscard]] ImVec2 GetCanvasPosition() const
        {
            return currentCanvasPos;
        }

        /**
         * @brief Returns canvas size.
         * @return Canvas size
         */
        [[nodiscard]] ImVec2 GetCanvasSize() const
        {
            return currentCanvasSize;
        }

        /**
         * @brief Returns zoom level.
         * @return Zoom level
         */
        [[nodiscard]] float GetZoomLevel() const
        {
            return zoomLevel;
        }

        /**
         * @brief Sets zoom level.
         * @param newZoom Target zoom level
         */
        void SetZoomLevel(float newZoom);

        /**
         * @brief Applies zoom delta.
         * @param delta Zoom change
         */
        void ApplyZoomDelta(float delta);

        /**
         * @brief Zooms to fit content.
         * @param contentMin Content minimum bounds
         * @param contentMax Content maximum bounds
         * @param padding Padding around content
         */
        void ZoomToFit(const ImVec2 &contentMin, const ImVec2 &contentMax, float padding = 50.0f);

        /**
         * @brief Returns pan offset.
         * @return Pan offset
         */
        [[nodiscard]] ImVec2 GetPanOffset() const
        {
            return ImVec2(panX, panY);
        }

        /**
         * @brief Sets pan offset.
         * @param offset Pan offset
         */
        void SetPanOffset(const ImVec2 &offset);

        /**
         * @brief Applies pan delta.
         * @param delta Pan change
         */
        void ApplyPanDelta(const ImVec2 &delta);

        /**
         * @brief Centers view on position.
         * @param worldPos World position to center on
         */
        void CenterOn(const ImVec2 &worldPos);

        /**
         * @brief Returns grid visibility.
         * @return True if grid is visible
         */
        [[nodiscard]] bool IsGridVisible() const
        {
            return showGrid;
        }

        /**
         * @brief Sets grid visibility.
         * @param visible Grid visibility
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
         * @brief Returns grid size.
         * @return Grid cell size
         */
        [[nodiscard]] float GetGridSize() const
        {
            return gridSize;
        }

        /**
         * @brief Sets grid size.
         * @param size Grid cell size
         */
        void SetGridSize(float size)
        {
            gridSize = size;
        }

        /**
         * @brief Resets canvas to default view.
         */
        void ResetView();

        /**
         * @brief Checks if rectangle is visible.
         * @param worldMin Top-left corner
         * @param worldMax Bottom-right corner
         * @return True if visible
         */
        bool IsRectangleVisible(const ImVec2 &worldMin, const ImVec2 &worldMax) const;

        /**
         * @brief Returns visible world bounds.
         * @param outMin Top-left visible world position
         * @param outMax Bottom-right visible world position
         */
        void GetVisibleWorldBounds(ImVec2 &outMin, ImVec2 &outMax) const;

    private:
        /**
         * @brief Renders grid background.
         */
        void RenderGrid();

        /**
         * @brief Clamps zoom to valid bounds.
         * @param zoom Input zoom level
         * @return Clamped zoom level
         */
        float ClampZoom(float zoom) const;

        /**
         * @brief Ensures minimum canvas size.
         * @param size Input canvas size
         * @return Size with minimum bounds
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

} // namespace VisionCraft::UI::Canvas
