#pragma once
#include "Node.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Engine
{

    /**
     * @brief Connection between two nodes.
     */
    struct Connection
    {
        NodeId from; ///< Source node ID
        NodeId to;   ///< Destination node ID
    };

    /**
     * @brief Manages nodes and their connections.
     */
    class NodeEditor
    {
    public:
        /**
         * @brief Construct a new NodeEditor object.
         */
        NodeEditor();

        /**
         * @brief Adds node to editor.
         * @param node Node to add
         * @return Assigned node ID
         */
        NodeId AddNode(NodePtr node);

        /**
         * @brief Removes node from editor.
         * @param id Node ID
         * @return True if removed
         */
        [[nodiscard]] bool RemoveNode(NodeId id);

        /**
         * @brief Returns pointer to node.
         * @param id Node ID
         * @return Pointer to node, or nullptr if not found
         */
        [[nodiscard]] Node *GetNode(NodeId id);

        /**
         * @brief Returns const pointer to node.
         * @param id Node ID
         * @return Const pointer to node, or nullptr if not found
         */
        [[nodiscard]] const Node *GetNode(NodeId id) const;

        /**
         * @brief Returns all node IDs.
         * @return Vector of node IDs
         */
        [[nodiscard]] std::vector<NodeId> GetNodeIds() const;

        /**
         * @brief Adds connection between nodes.
         * @param from Source node ID
         * @param to Destination node ID
         */
        void AddConnection(NodeId from, NodeId to);

        /**
         * @brief Removes connection between nodes.
         * @param from Source node ID
         * @param to Destination node ID
         * @return True if removed
         */
        [[nodiscard]] bool RemoveConnection(NodeId from, NodeId to);

        /**
         * @brief Returns all connections.
         * @return Vector of connections
         */
        [[nodiscard]] const std::vector<Connection> &GetConnections() const;

        /**
         * @brief Removes all nodes and connections.
         */
        void Clear();

        /**
         * @brief Executes node graph in dependency order.
         * @return True if succeeded, false if cycle detected
         * @note Performs topological sort, passes data between nodes, and calls Process() in order.
         */
        bool Execute();

        /**
         * @brief Serializes graph to JSON file.
         * @param filepath Path to save file
         * @param nodePositions Map of node IDs to positions (x,y coordinates)
         * @return True if succeeded
         */
        bool SaveToFile(const std::filesystem::path &filepath,
            const std::unordered_map<NodeId, std::pair<float, float>> &nodePositions) const;

        /**
         * @brief Deserializes graph from JSON file.
         * @param filepath Path to load file
         * @param nodePositions Output map for node positions
         * @return True if succeeded
         */
        bool LoadFromFile(const std::filesystem::path &filepath,
            std::unordered_map<NodeId, std::pair<float, float>> &nodePositions);

    private:
        /**
         * @brief Performs topological sort on node graph.
         * @return Node IDs in execution order, or empty if cycle detected
         */
        [[nodiscard]] std::vector<NodeId> TopologicalSort() const;

        /**
         * @brief Passes data between nodes using slot system.
         * @param fromNode Source node
         * @param toNode Destination node
         */
        static void PassDataBetweenNodes(Node *fromNode, Node *toNode);
        std::unordered_map<NodeId, NodePtr> nodes; ///< Node storage
        std::vector<Connection> connections;       ///< Connections
        NodeId nextId;                             ///< Next available ID
    };

} // namespace VisionCraft::Engine
