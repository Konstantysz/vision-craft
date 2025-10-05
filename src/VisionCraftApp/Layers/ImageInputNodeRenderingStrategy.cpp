#include "ImageInputNodeRenderingStrategy.h"
#include "ConnectionManager.h"
#include "NodeDimensionCalculator.h"
#include "NodeEditorConstants.h"
#include "Nodes/ImageInputNode.h"
#include <algorithm>

namespace VisionCraft
{
    void ImageInputNodeRenderingStrategy::RenderCustomContent(Engine::Node &node,
        const ImVec2 &nodePos,
        const ImVec2 &nodeSize,
        float zoomLevel)
    {
        auto &imageNode = static_cast<Engine::ImageInputNode &>(node);
        if (!imageNode.HasValidImage() || imageNode.GetTextureId() == 0)
        {
            return;
        }

        const float titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        const float padding = Constants::Node::kPadding * zoomLevel;

        const auto pins = ConnectionManager::GetNodePins(node.GetName());
        std::vector<NodePin> inputPins, outputPins;
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) { return pin.isInput; });
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const float parameterAreaHeight =
            NodeDimensionCalculator::CalculateBaseContentHeight(inputPins, outputPins, zoomLevel) - (padding * 2);

        const float extraSpacing = 10.0f * zoomLevel;
        const float previewY = nodePos.y + titleHeight + padding + parameterAreaHeight + extraSpacing;
        const float nodeContentWidth = nodeSize.x - (padding * 2);
        auto [previewWidth, previewHeight] = imageNode.CalculatePreviewDimensions(nodeContentWidth, 0);

        if (previewWidth > 0 && previewHeight > 0)
        {
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + padding, previewY));

            ImGui::Image(static_cast<ImTextureID>(imageNode.GetTextureId()),
                ImVec2(previewWidth, previewHeight),
                ImVec2(0, 0),
                ImVec2(1, 1));

            if (ImGui::IsItemHovered())
            {
                auto outputImage = imageNode.GetOutputImage();
                if (outputImage.rows > 0 && outputImage.cols > 0)
                {
                    float imageAspect = static_cast<float>(outputImage.cols) / static_cast<float>(outputImage.rows);
                    ImGui::SetTooltip(
                        "%dx%d pixels\nAspect ratio: %.2f", outputImage.cols, outputImage.rows, imageAspect);
                }
            }
        }
    }

} // namespace VisionCraft