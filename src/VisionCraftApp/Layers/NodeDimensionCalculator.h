#pragma once

#include "Node.h"
#include "NodeEditorTypes.h"
#include <vector>

namespace VisionCraft
{
    /**
     * @brief Utility for calculating node dimensions.
     */
    class NodeDimensionCalculator
    {
    public:
        /**
         * @brief Calculates pin column height.
         * @param pins Pins
         * @param zoomLevel Zoom level
         * @return Column height
         */
        [[nodiscard]] static float CalculatePinColumnHeight(const std::vector<NodePin> &pins, float zoomLevel);

        /**
         * @brief Calculates base content height.
         * @param inputPins Input pins
         * @param outputPins Output pins
         * @param zoomLevel Zoom level
         * @return Base content height
         */
        [[nodiscard]] static float CalculateBaseContentHeight(const std::vector<NodePin> &inputPins,
            const std::vector<NodePin> &outputPins,
            float zoomLevel);

        /**
         * @brief Calculates node dimensions.
         * @param pins Pins
         * @param zoomLevel Zoom level
         * @param node Node
         * @return Node dimensions
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