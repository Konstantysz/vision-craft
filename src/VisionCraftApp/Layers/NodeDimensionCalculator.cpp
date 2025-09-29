#include "NodeDimensionCalculator.h"
#include "NodeEditorConstants.h"
#include <algorithm>

namespace VisionCraft
{
    float NodeDimensionCalculator::CalculatePinColumnHeight(const std::vector<NodePin> &pins, float zoomLevel)
    {
        const auto compactPinHeight = Constants::Pin::kCompactHeight * zoomLevel;
        const auto extendedPinHeight = Constants::Pin::kHeight * zoomLevel;
        const auto compactSpacing = Constants::Pin::kCompactSpacing * zoomLevel;
        const auto normalSpacing = Constants::Pin::kSpacing * zoomLevel;

        float totalHeight = 0;
        for (const auto &pin : pins)
        {
            const bool needsInputWidget = pin.dataType != PinDataType::Image;
            const auto pinHeight = needsInputWidget ? extendedPinHeight : compactPinHeight;
            const auto spacing = needsInputWidget ? normalSpacing : compactSpacing;
            totalHeight += pinHeight + spacing;
        }

        return totalHeight;
    }

    float NodeDimensionCalculator::CalculateBaseContentHeight(const std::vector<NodePin> &inputPins,
        const std::vector<NodePin> &outputPins,
        float zoomLevel)
    {
        const auto padding = Constants::Node::kPadding * zoomLevel;

        const auto inputColumnHeight = CalculatePinColumnHeight(inputPins, zoomLevel);
        const auto outputColumnHeight = CalculatePinColumnHeight(outputPins, zoomLevel);

        const auto pinsHeight = std::max(inputColumnHeight, outputColumnHeight);
        return pinsHeight + padding * 2;
    }

    NodeDimensions NodeDimensionCalculator::CalculateNodeDimensions(const std::vector<NodePin> &pins,
        float zoomLevel,
        const Engine::Node *node)
    {
        std::vector<NodePin> inputPins, outputPins;

        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) { return pin.isInput; });

        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        const auto padding = Constants::Node::kPadding * zoomLevel;

        const auto contentHeight = CalculateBaseContentHeight(inputPins, outputPins, zoomLevel);

        float extraHeight = 0;
        if (node)
        {
            const float nodeWidth = Constants::Node::kWidth * zoomLevel;
            const float nodeContentWidth = nodeWidth - (padding * 2);
            extraHeight = node->CalculateExtraHeight(nodeContentWidth, zoomLevel);
        }

        const auto totalHeight = titleHeight + contentHeight + extraHeight + padding;

        const float nodeWidth = Constants::Node::kWidth * zoomLevel;
        return { ImVec2(nodeWidth, std::max(Constants::Node::kMinHeight * zoomLevel, totalHeight)),
            inputPins.size(),
            outputPins.size(),
            0 };
    }

} // namespace VisionCraft