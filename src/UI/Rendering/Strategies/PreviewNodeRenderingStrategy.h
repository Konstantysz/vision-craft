#pragma once

#include "Rendering/Strategies/NodeRenderingStrategy.h"

namespace VisionCraft
{
    /**
     * @brief Rendering strategy for PreviewNode with image display.
     */
    class PreviewNodeRenderingStrategy : public NodeRenderingStrategy
    {
    public:
        void RenderCustomContent(Engine::Node &node,
            const ImVec2 &nodePos,
            const ImVec2 &nodeSize,
            float zoomLevel) override;
    };

} // namespace VisionCraft
