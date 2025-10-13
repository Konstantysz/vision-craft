#include "Rendering/Strategies/PreviewNodeRenderingStrategy.h"
#include "Connections/ConnectionManager.h"
#include "Editor/NodeEditorConstants.h"
#include "Rendering/NodeDimensionCalculator.h"
#include "Nodes/PreviewNode.h"
#include <algorithm>

namespace VisionCraft
{
    void PreviewNodeRenderingStrategy::RenderCustomContent(Engine::Node &node,
        const ImVec2 &nodePos,
        const ImVec2 &nodeSize,
        float zoomLevel)
    {
        auto &previewNode = static_cast<Engine::PreviewNode &>(node);
        if (!previewNode.HasValidImage() || previewNode.GetTextureId() == 0)
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
        auto [previewWidth, previewHeight] = previewNode.CalculatePreviewDimensions(nodeContentWidth, 0);

        if (previewWidth > 0 && previewHeight > 0)
        {
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + padding, previewY));

            ImGui::Image(static_cast<ImTextureID>(previewNode.GetTextureId()),
                ImVec2(previewWidth, previewHeight),
                ImVec2(0, 0),
                ImVec2(1, 1));

            if (ImGui::IsItemHovered())
            {
                auto outputImage = previewNode.GetOutputImage();
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
