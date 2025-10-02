#pragma once

#include "Node.h"
#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Strategy for node-specific content rendering.
     */
    class NodeRenderingStrategy
    {
    public:
        virtual ~NodeRenderingStrategy() = default;

        /**
         * @brief Renders custom node content.
         * @param node Node
         * @param nodePos Node position
         * @param nodeSize Node size
         * @param zoomLevel Zoom level
         */
        virtual void
            RenderCustomContent(Engine::Node &node, const ImVec2 &nodePos, const ImVec2 &nodeSize, float zoomLevel) = 0;
    };

} // namespace VisionCraft