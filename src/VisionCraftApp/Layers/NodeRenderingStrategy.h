#pragma once

#include "Node.h"
#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Strategy interface for node-specific content rendering.
     *
     * Simple strategy pattern focused only on custom content rendering.
     * Follows KISS principle - add methods only when actually needed.
     */
    class NodeRenderingStrategy
    {
    public:
        virtual ~NodeRenderingStrategy() = default;

        /**
         * @brief Renders custom content for this node type.
         * @param node The node to render content for
         * @param nodePos Position of the node
         * @param nodeSize Size of the node
         * @param zoomLevel Current zoom level
         */
        virtual void
            RenderCustomContent(Engine::Node &node, const ImVec2 &nodePos, const ImVec2 &nodeSize, float zoomLevel) = 0;
    };

} // namespace VisionCraft