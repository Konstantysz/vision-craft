#pragma once

#include "UI/Widgets/NodeEditorTypes.h"
#include "Nodes/Core/Node.h"

#include <vector>

namespace VisionCraft::UI::Rendering
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
        [[nodiscard]] static float CalculatePinColumnHeight(const std::vector<UI::Widgets::NodePin> &pins,
            float zoomLevel);

        /**
         * @brief Calculates base content height.
         * @param inputPins Input pins
         * @param outputPins Output pins
         * @param zoomLevel Zoom level
         * @return Base content height
         */
        [[nodiscard]] static float CalculateBaseContentHeight(const std::vector<UI::Widgets::NodePin> &inputPins,
            const std::vector<UI::Widgets::NodePin> &outputPins,
            float zoomLevel);

        /**
         * @brief Calculates node dimensions.
         * @param pins Pins
         * @param zoomLevel Zoom level
         * @param node Nodes::Node
         * @return Nodes::Node dimensions
         */
        [[nodiscard]] static UI::Widgets::NodeDimensions CalculateNodeDimensions(
            const std::vector<UI::Widgets::NodePin> &pins,
            float zoomLevel,
            const Nodes::Node *node = nullptr);

    private:
        NodeDimensionCalculator() = delete;
        ~NodeDimensionCalculator() = delete;
        NodeDimensionCalculator(const NodeDimensionCalculator &) = delete;
        NodeDimensionCalculator &operator=(const NodeDimensionCalculator &) = delete;
    };

} // namespace VisionCraft::UI::Rendering
