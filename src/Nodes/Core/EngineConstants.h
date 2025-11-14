#pragma once

#include <cstddef>

/**
 * @file EngineConstants.h
 * @brief Contains constants used by the VisionCraft Engine layer.
 *
 * This file centralizes Engine-specific constants, keeping them separate
 * from App layer constants to maintain proper layering architecture.
 */

namespace VisionCraft::Constants
{
    /**
     * @brief Buffer and storage constants for Engine layer.
     */
    namespace Buffers
    {
        /// @brief Buffer size for file path input widgets
        constexpr size_t kFilePathBufferSize = 512;
    } // namespace Buffers

    /**
     * @brief ImageInputNode-specific constants.
     */
    namespace ImageInputNode
    {
        /**
         * @brief UI layout constants for ImageInputNode.
         */
        namespace UI
        {
            /// @brief Width of Browse and Load buttons
            constexpr float kButtonWidth = 55.0f;

            /// @brief Spacing between UI elements
            constexpr float kSpacing = 5.0f;
        } // namespace UI

        /**
         * @brief Image preview constants.
         */
        namespace Preview
        {
            /// @brief Maximum width for compact image preview
            constexpr float kMaxWidth = 120.0f;

            /// @brief Maximum height for compact image preview
            constexpr float kMaxHeight = 80.0f;

            /// @brief Spacing around image preview
            constexpr float kSpacing = 10.0f;

            /// @brief Center alignment factor (0.5 = 50%)
            constexpr float kCenterAlignFactor = 0.5f;
        } // namespace Preview

        /**
         * @brief Status text colors (ImVec4 format).
         */
        namespace StatusColors
        {
            /// @brief Success color (green) for loaded image
            constexpr float kSuccessColor[4] = { 0.0f, 0.8f, 0.0f, 1.0f };

            /// @brief Warning color (yellow) for no image
            constexpr float kWarningColor[4] = { 0.8f, 0.8f, 0.0f, 1.0f };
        } // namespace StatusColors
    }     // namespace ImageInputNode

} // namespace VisionCraft::Constants
