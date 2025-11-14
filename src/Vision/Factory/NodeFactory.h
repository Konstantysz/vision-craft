#pragma once

#include "Nodes/Core/Node.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Vision
{
    /**
     * @brief Factory for creating nodes using the registry pattern.
     *
     * This class follows the Open/Closed Principle - new node types can be added
     * without modifying existing code by registering them with the factory.
     */
    class NodeFactory
    {
    public:
        /**
         * @brief Function signature for node creators.
         * @param id Node ID
         * @param name Node display name
         * @return Unique pointer to created node
         */
        using NodeCreator = std::function<std::unique_ptr<Nodes::Node>(Nodes::NodeId id, std::string_view name)>;

        /**
         * @brief Registers a node type with the factory.
         * @param type Node type identifier
         * @param creator Function that creates the node
         */
        static void Register(std::string_view type, NodeCreator creator);

        /**
         * @brief Creates a node of the specified type.
         * @param type Node type identifier
         * @param id Node ID
         * @param name Node display name
         * @return Unique pointer to created node, or nullptr if type not found
         */
        [[nodiscard]] static std::unique_ptr<Nodes::Node>
            CreateNode(std::string_view type, Nodes::NodeId id, std::string_view name);

        /**
         * @brief Checks if a node type is registered.
         * @param type Node type identifier
         * @return True if type is registered
         */
        [[nodiscard]] static bool IsRegistered(std::string_view type);

        /**
         * @brief Returns all registered node types.
         * @return Vector of registered type names
         */
        [[nodiscard]] static std::vector<std::string> GetRegisteredTypes();

        /**
         * @brief Registers all available node types.
         * @note This must be called before using the factory.
         */
        static void RegisterAllNodes();

    private:
        static std::unordered_map<std::string, NodeCreator> &GetRegistry();
    };
} // namespace VisionCraft::Vision
