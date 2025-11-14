#include "UI/Rendering/Strategies/ImageInputNodeRenderingStrategy.h"
#include "UI/Canvas/ConnectionManager.h"
#include "UI/Rendering/NodeDimensionCalculator.h"
#include "UI/Widgets/NodeEditorConstants.h"
#include "Vision/IO/ImageInputNode.h"
#include <algorithm>

namespace VisionCraft::UI::Rendering::Strategies
{
    namespace
    {
        constexpr float kExtraSpacing = 10.0f;
        constexpr ImU32 kErrorColor = IM_COL32(255, 76, 76, 255);

        struct ContentAreaInfo
        {
            float parameterAreaHeight;
            float contentY;
        };

        ContentAreaInfo CalculateContentArea(const Nodes::Node &node, const ImVec2 &nodePos, float zoomLevel)
        {
            const float titleHeight = Constants::Node::kTitleHeight * zoomLevel;
            const float padding = Constants::Node::kPadding * zoomLevel;

            const auto pins = Canvas::ConnectionManager::GetNodePins(node.GetName());
            std::vector<Widgets::NodePin> inputPins, outputPins;
            std::copy_if(
                pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) { return pin.isInput; });
            std::copy_if(
                pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

            const float parameterAreaHeight =
                NodeDimensionCalculator::CalculateBaseContentHeight(inputPins, outputPins, zoomLevel) - (padding * 2);

            const float contentY =
                nodePos.y + titleHeight + padding + parameterAreaHeight + (kExtraSpacing * zoomLevel);

            return { parameterAreaHeight, contentY };
        }
    } // anonymous namespace

    void ImageInputNodeRenderingStrategy::RenderCustomContent(Nodes::Node &node,
        const ImVec2 &nodePos,
        const ImVec2 &nodeSize,
        float zoomLevel)
    {
        auto &imageNode = static_cast<Vision::IO::ImageInputNode &>(node);

        const float padding = Constants::Node::kPadding * zoomLevel;
        const auto [parameterAreaHeight, contentY] = CalculateContentArea(node, nodePos, zoomLevel);

        // Show error message if present
        if (imageNode.HasError())
        {
            auto *drawList = ImGui::GetWindowDrawList();
            const ImVec2 errorPos = ImVec2(nodePos.x + padding, contentY);
            drawList->AddText(
                ImGui::GetFont(), ImGui::GetFontSize(), errorPos, kErrorColor, imageNode.GetErrorMessage().c_str());
            return;
        }

        if (!imageNode.HasValidImage() || imageNode.GetTextureId() == 0)
        {
            return;
        }

        const float nodeContentWidth = nodeSize.x - (padding * 2);
        auto [previewWidth, previewHeight] = imageNode.CalculatePreviewDimensions(nodeContentWidth, 0);

        if (previewWidth > 0 && previewHeight > 0)
        {
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + padding, contentY));

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

} // namespace VisionCraft::UI::Rendering::Strategies
