#pragma once

#include "UI/Rendering/Strategies/NodeRenderingStrategy.h"

namespace VisionCraft::UI::Rendering::Strategies
{
    /**
     * @brief Rendering strategy for PreviewNode with image display.
     */
    class PreviewNodeRenderingStrategy : public NodeRenderingStrategy
    {
    public:
        void RenderCustomContent(Nodes::Node &node,
            const ImVec2 &nodePos,
            const ImVec2 &nodeSize,
            float zoomLevel) override;
    };

} // namespace VisionCraft::UI::Rendering::Strategies
