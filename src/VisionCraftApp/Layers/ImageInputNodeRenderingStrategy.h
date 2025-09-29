#pragma once

#include "NodeRenderingStrategy.h"

namespace VisionCraft
{
    /**
     * @brief Rendering strategy for ImageInputNode with image preview.
     */
    class ImageInputNodeRenderingStrategy : public NodeRenderingStrategy
    {
    public:
        void RenderCustomContent(Engine::Node &node,
            const ImVec2 &nodePos,
            const ImVec2 &nodeSize,
            float zoomLevel) override;
    };

} // namespace VisionCraft