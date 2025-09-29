#include "ImageInputNodeRenderingStrategy.h"
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

        // Calculate actual parameter area height (same logic as was in RenderCustomNodeContent)
        const auto compactPinHeight = Constants::Pin::kCompactHeight * zoomLevel;
        const auto extendedPinHeight = Constants::Pin::kHeight * zoomLevel;
        const auto compactSpacing = Constants::Pin::kCompactSpacing * zoomLevel;
        const auto normalSpacing = Constants::Pin::kSpacing * zoomLevel;

        // We need to calculate the parameter height, but this requires pin information
        // For now, let's use a reasonable estimate based on ImageInputNode's known pins
        const float estimatedParameterHeight = extendedPinHeight + normalSpacing; // filepath pin

        // Add spacing to ensure the image appears clearly below parameters
        const float extraSpacing = 10.0f * zoomLevel;
        const float previewY = nodePos.y + titleHeight + padding + estimatedParameterHeight + extraSpacing;

        // Calculate full-width preview size
        const float nodeContentWidth = nodeSize.x - (padding * 2);
        auto [previewWidth, previewHeight] = imageNode.CalculatePreviewDimensions(nodeContentWidth, 0);

        if (previewWidth > 0 && previewHeight > 0)
        {
            // Always position at left edge with padding
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + padding, previewY));

            ImGui::Image(static_cast<ImTextureID>(imageNode.GetTextureId()),
                ImVec2(previewWidth, previewHeight),
                ImVec2(0, 0),
                ImVec2(1, 1));

            // Show tooltip with image info on hover
            if (ImGui::IsItemHovered())
            {
                auto outputImage = imageNode.GetOutputImage();
                float imageAspect = static_cast<float>(outputImage.cols) / static_cast<float>(outputImage.rows);
                ImGui::SetTooltip("%dx%d pixels\nAspect ratio: %.2f", outputImage.cols, outputImage.rows, imageAspect);
            }
        }
    }

} // namespace VisionCraft