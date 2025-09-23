#pragma once

#include <imgui.h>

/**
 * @file NodeEditorConstants.h
 * @brief Contains all constants used throughout the node editor system.
 *
 * This file centralizes all magic numbers, colors, and configuration values
 * used in the node editor to improve maintainability and enable easy theming.
 */

namespace VisionCraft::Constants
{
    // ========================================
    // ZOOM & CANVAS CONSTANTS
    // ========================================

    /**
     * @brief Zoom-related constants for canvas navigation.
     */
    namespace Zoom
    {
        /// @brief Default zoom level (100%)
        constexpr float kDefault = 1.0f;

        /// @brief Minimum allowed zoom level (10%)
        constexpr float kMin = 0.1f;

        /// @brief Maximum allowed zoom level (500%)
        constexpr float kMax = 5.0f;

        /// @brief Zoom step size for mouse wheel events
        constexpr float kStep = 0.1f;

        /// @brief Minimum zoom level at which text is rendered (performance optimization)
        constexpr float kMinForText = 0.5f;
    }

    /**
     * @brief Canvas and grid rendering constants.
     */
    namespace Canvas
    {
        /// @brief Minimum canvas size in pixels to prevent rendering issues
        constexpr float kMinSize = 50.0f;

        /// @brief Size of each grid cell in pixels
        constexpr float kGridSize = 20.0f;

        /// @brief Alpha transparency value for grid lines (0-255)
        constexpr int kGridAlpha = 40;
    }

    // ========================================
    // NODE VISUAL CONSTANTS
    // ========================================

    /**
     * @brief Node appearance and sizing constants.
     */
    namespace Node
    {
        /// @brief Default width of nodes in pixels
        constexpr float kWidth = 220.0f;

        /// @brief Minimum height of nodes in pixels
        constexpr float kMinHeight = 100.0f;

        /// @brief Height of the node title bar in pixels
        constexpr float kTitleHeight = 25.0f;

        /// @brief Internal padding within nodes in pixels
        constexpr float kPadding = 8.0f;

        /// @brief Corner rounding radius for node rectangles in pixels
        constexpr float kRounding = 8.0f;

        /**
         * @brief Node border thickness constants.
         */
        namespace Border
        {
            /// @brief Border thickness for unselected nodes in pixels
            constexpr float kThicknessNormal = 2.0f;

            /// @brief Border thickness for selected nodes in pixels
            constexpr float kThicknessSelected = 3.0f;
        }

        /**
         * @brief Node text rendering constants.
         */
        namespace Text
        {
            /// @brief Text offset from node edges in pixels
            constexpr float kOffset = 4.0f;
        }

        /**
         * @brief Node creation positioning constants.
         */
        namespace Creation
        {
            /// @brief Horizontal offset when creating nodes at cursor position
            constexpr float kOffsetX = 100.0f;

            /// @brief Vertical offset when creating nodes at cursor position
            constexpr float kOffsetY = 40.0f;
        }
    }

    // ========================================
    // PIN CONSTANTS
    // ========================================

    /**
     * @brief Pin appearance and layout constants.
     */
    namespace Pin
    {
        /// @brief Height of pin rows in pixels
        constexpr float kHeight = 18.0f;

        /// @brief Vertical spacing between pins in pixels
        constexpr float kSpacing = 3.0f;

        /// @brief Radius of pin circles in pixels
        constexpr float kRadius = 5.0f;

        /// @brief Thickness of pin border circles in pixels
        constexpr float kBorderThickness = 1.5f;

        /// @brief Horizontal offset between pin circle and label text
        constexpr float kTextOffset = 4.0f;
    }

    /**
     * @brief Parameter input widget constants.
     */
    namespace Parameter
    {
        /// @brief Height of parameter input rows in pixels
        constexpr float kHeight = 22.0f;

        /// @brief Minimum width for parameter input widgets
        constexpr float kMinInputWidth = 80.0f;

        /// @brief Vertical offset for parameter labels
        constexpr float kLabelOffset = 2.0f;

        /// @brief Vertical offset for parameter input widgets
        constexpr float kInputOffset = 12.0f;
    }

    // ========================================
    // CONNECTION CONSTANTS
    // ========================================

    /**
     * @brief Connection line rendering constants.
     */
    namespace Connection
    {
        /// @brief Thickness of connection lines in pixels
        constexpr float kThickness = 3.0f;

        /// @brief Bezier curve tension for connection curves
        constexpr float kBezierTension = 100.0f;
    }

    // ========================================
    // COLOR CONSTANTS
    // ========================================

    /**
     * @brief Color definitions for all node editor elements.
     *
     * Colors follow UE Blueprints conventions where applicable.
     * Format: IM_COL32(red, green, blue, alpha) with values 0-255.
     */
    namespace Colors
    {
        /**
         * @brief Node visual colors.
         */
        namespace Node
        {
            /// @brief Background color for node bodies
            constexpr ImU32 kBackground = IM_COL32(60, 60, 60, 255);

            /// @brief Background color for node title bars
            constexpr ImU32 kTitle = IM_COL32(80, 80, 120, 255);

            /// @brief Border color for unselected nodes
            constexpr ImU32 kBorderNormal = IM_COL32(100, 100, 100, 255);

            /// @brief Border color for selected nodes (orange highlight)
            constexpr ImU32 kBorderSelected = IM_COL32(255, 165, 0, 255);

            /// @brief Text color for node titles
            constexpr ImU32 kText = IM_COL32(255, 255, 255, 255);
        }

        /**
         * @brief Pin visual colors.
         */
        namespace Pin
        {
            /// @brief Border color for pin circles
            constexpr ImU32 kBorder = IM_COL32(255, 255, 255, 255);

            /// @brief Text color for pin labels
            constexpr ImU32 kLabel = IM_COL32(200, 200, 200, 255);

            // Data type colors following UE Blueprints convention

            /// @brief Color for Image data type pins (green)
            constexpr ImU32 kImage = IM_COL32(100, 200, 100, 255);

            /// @brief Color for String data type pins (magenta)
            constexpr ImU32 kString = IM_COL32(200, 100, 200, 255);

            /// @brief Color for Float data type pins (light blue)
            constexpr ImU32 kFloat = IM_COL32(100, 150, 255, 255);

            /// @brief Color for Int data type pins (cyan)
            constexpr ImU32 kInt = IM_COL32(100, 255, 255, 255);

            /// @brief Color for Bool data type pins (red)
            constexpr ImU32 kBool = IM_COL32(255, 100, 100, 255);

            /// @brief Default color for unknown data types (gray)
            constexpr ImU32 kDefault = IM_COL32(128, 128, 128, 255);
        }

        /**
         * @brief Connection line colors.
         */
        namespace Connection
        {
            /// @brief Color for established connections
            constexpr ImU32 kActive = IM_COL32(150, 150, 150, 255);

            /// @brief Color for connection being created (preview)
            constexpr ImU32 kCreating = IM_COL32(200, 200, 200, 255);
        }

        /**
         * @brief Grid rendering colors.
         */
        namespace Grid
        {
            /// @brief Color for grid lines (uses Canvas::kGridAlpha for transparency)
            constexpr ImU32 kLines = IM_COL32(200, 200, 200, Canvas::kGridAlpha);
        }
    }

    // ========================================
    // SPECIAL CONSTANTS
    // ========================================

    /**
     * @brief Special values and identifiers.
     */
    namespace Special
    {
        /// @brief Sentinel value indicating no valid node is selected
        constexpr int kInvalidNodeId = -1;

        /// @brief Buffer size for string parameter input widgets
        constexpr size_t kStringBufferSize = 256;
    }

    // ========================================
    // INPUT CONSTANTS
    // ========================================

    /**
     * @brief Input widget behavior constants.
     */
    namespace Input
    {
        /**
         * @brief Float input widget settings.
         */
        namespace Float
        {
            /// @brief Small step size for float inputs
            constexpr float kStep = 0.1f;

            /// @brief Large step size for float inputs
            constexpr float kFastStep = 1.0f;

            /// @brief Display format for float values
            constexpr const char* kFormat = "%.2f";
        }
    }

} // namespace VisionCraft::Constants