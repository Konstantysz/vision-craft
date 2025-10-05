#pragma once

#include "Node.h"
#include <memory>
#include <string>

namespace VisionCraft::Engine
{
    /**
     * @brief Factory for creating nodes from type strings.
     */
    class NodeFactory
    {
    public:
        /**
         * @brief Creates node instance from type string.
         * @param type Node type identifier
         * @param id Node ID to assign
         * @param name Node display name
         * @return Unique pointer to created node, or nullptr if type unknown
         */
        static NodePtr CreateNode(const std::string &type, NodeId id, const std::string &name);
    };
} // namespace VisionCraft::Engine
