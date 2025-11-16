#pragma once

#include "Nodes/Core/Node.h"

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Vision
{
    /**
     * @brief Concept to ensure a type is a valid Node type.
     *
     * This C++20 concept enforces that any type registered with the NodeFactory
     * must derive from the Node base class. This provides compile-time safety
     * when creating node types, ensuring type errors are caught early.
     *
     * @tparam T Type to check
     *
     * Requirements:
     * - T must be derived from VisionCraft::Nodes::Node
     * - T must have a valid constructor taking (NodeId, std::string_view)
     *
     * Example valid types: ImageInputNode, GrayscaleNode, ThresholdNode
     * Example invalid types: int, std::string, custom classes not derived from Node
     */
    template<typename T>
    concept NodeType = std::derived_from<T, Nodes::Node> && requires(Nodes::NodeId id, std::string_view name) {
        // T must be constructible with NodeId and string_view
        {
            T(id, name)
        } -> std::convertible_to<T>;
    };

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
         * @brief Registers a node type with compile-time type safety (C++20 concepts).
         *
         * This template helper uses C++20 concepts to ensure the registered type
         * is actually a valid Node subclass at compile time. It eliminates the need
         * for manual lambda writing and provides better error messages.
         *
         * @tparam T Node type to register (must satisfy NodeType concept)
         * @param type Node type identifier string
         *
         * Example usage:
         * @code
         * NodeFactory::RegisterNode<ImageInputNode>("ImageInput");
         * NodeFactory::RegisterNode<GrayscaleNode>("Grayscale");
         * @endcode
         */
        template<NodeType T> static void RegisterNode(std::string_view type)
        {
            Register(type,
                [](Nodes::NodeId id, std::string_view name) { return std::make_unique<T>(id, std::string(name)); });
        }

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
