#pragma once

#include "NodeRenderingStrategy.h"

namespace VisionCraft
{
    /**
     * @brief Default strategy - no custom content.
     */
    class DefaultNodeRenderingStrategy : public NodeRenderingStrategy
    {
    public:
        void RenderCustomContent(Engine::Node &node,
            const ImVec2 &nodePos,
            const ImVec2 &nodeSize,
            float zoomLevel) override;
    };

} // namespace VisionCraft