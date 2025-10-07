#pragma once

#include "Node.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace VisionCraft
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
        using NodeCreator = std::function<std::unique_ptr<Engine::Node>(Engine::NodeId id, std::string_view name)>;

        /**
         * @brief Registers a node type with the factory.
         * @param type Node type identifier
         * @param creator Function that creates the node
         */
        void Register(std::string_view type, NodeCreator creator);

        /**
         * @brief Creates a node of the specified type.
         * @param type Node type identifier
         * @param id Node ID
         * @param name Node display name
         * @return Unique pointer to created node, or nullptr if type not found
         */
        [[nodiscard]] std::unique_ptr<Engine::Node>
            Create(std::string_view type, Engine::NodeId id, std::string_view name) const;

        /**
         * @brief Checks if a node type is registered.
         * @param type Node type identifier
         * @return True if type is registered
         */
        [[nodiscard]] bool IsRegistered(std::string_view type) const;

        /**
         * @brief Returns all registered node types.
         * @return Vector of registered type names
         */
        [[nodiscard]] std::vector<std::string> GetRegisteredTypes() const;

    private:
        std::unordered_map<std::string, NodeCreator> registry;
    };
} // namespace VisionCraft
