#pragma once

#include "Node.h"
#include "NodeEditorTypes.h"
#include <vector>

namespace VisionCraft
{
    /**
     * @brief Utility class for calculating node dimensions consistently across the application.
     */
    class NodeDimensionCalculator
    {
    public:
        /**
         * @brief Calculates pin column height for input or output pins.
         * @param pins Vector of pins to calculate height for
         * @param zoomLevel Current zoom level
         * @return Total height needed for the pin column
         */
        [[nodiscard]] static float CalculatePinColumnHeight(const std::vector<NodePin> &pins, float zoomLevel);

        /**
         * @brief Calculates the base content height (pins + padding).
         * @param inputPins Input pins for the node
         * @param outputPins Output pins for the node
         * @param zoomLevel Current zoom level
         * @return Base content height without extra node-specific content
         */
        [[nodiscard]] static float CalculateBaseContentHeight(const std::vector<NodePin> &inputPins,
            const std::vector<NodePin> &outputPins,
            float zoomLevel);

        /**
         * @brief Calculates complete node dimensions.
         * @param pins Vector of all pins for this node
         * @param zoomLevel Current zoom level
         * @param node Pointer to the node (for node-specific sizing)
         * @return Complete calculated node dimensions
         */
        [[nodiscard]] static NodeDimensions CalculateNodeDimensions(const std::vector<NodePin> &pins,
            float zoomLevel,
            const Engine::Node *node = nullptr);

    private:
        NodeDimensionCalculator() = delete;
        ~NodeDimensionCalculator() = delete;
        NodeDimensionCalculator(const NodeDimensionCalculator &) = delete;
        NodeDimensionCalculator &operator=(const NodeDimensionCalculator &) = delete;
    };

} // namespace VisionCraft