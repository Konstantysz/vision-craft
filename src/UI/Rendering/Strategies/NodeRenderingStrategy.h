#pragma once

#include "Nodes/Core/Node.h"
#include <imgui.h>

namespace VisionCraft::UI::Rendering::Strategies
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
         * @param node Nodes::Node
         * @param nodePos Nodes::Node position
         * @param nodeSize Nodes::Node size
         * @param zoomLevel Zoom level
         */
        virtual void
            RenderCustomContent(Nodes::Node &node, const ImVec2 &nodePos, const ImVec2 &nodeSize, float zoomLevel) = 0;
    };

} // namespace VisionCraft::UI::Rendering::Strategies
