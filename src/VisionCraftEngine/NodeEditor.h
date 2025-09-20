#pragma once
#include "Node.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Engine
{

    /**
     * @brief Structure representing a connection between two nodes.
     *
     * Holds the IDs of the source and destination nodes.
     */
    struct Connection
    {
        NodeId from; ///< ID of the source node
        NodeId to;   ///< ID of the destination node
    };

    /**
     * @brief Manages nodes and their connections in the editor backend.
     *
     * NodeEditor is responsible for storing, adding, removing, and connecting nodes.
     * It provides an API for manipulating the node graph and is independent of any GUI framework.
     */
    class NodeEditor
    {
    public:
        /**
         * @brief Construct a new NodeEditor object.
         */
        NodeEditor();

        /**
         * @brief Add a node to the editor.
         * @param node Unique pointer to the node to add.
         * @return NodeId The assigned ID of the node.
         */
        NodeId AddNode(NodePtr node);

        /**
         * @brief Remove a node from the editor.
         * @param id ID of the node to remove.
         * @return true if the node was removed, false otherwise.
         */
        bool RemoveNode(NodeId id);

        /**
         * @brief Get a pointer to a node by its ID.
         * @param id ID of the node.
         * @return Node* Pointer to the node, or nullptr if not found.
         */
        Node *GetNode(NodeId id);

        /**
         * @brief Get a const pointer to a node by its ID.
         * @param id ID of the node.
         * @return const Node* Const pointer to the node, or nullptr if not found.
         */
        const Node *GetNode(NodeId id) const;

        /**
         * @brief Get a vector of all node IDs in the editor.
         * @return std::vector<NodeId> Vector of node IDs.
         */
        std::vector<NodeId> GetNodeIds() const;

        /**
         * @brief Add a connection between two nodes.
         * @param from ID of the source node.
         * @param to ID of the destination node.
         */
        void AddConnection(NodeId from, NodeId to);

        /**
         * @brief Remove a connection between two nodes.
         * @param from ID of the source node.
         * @param to ID of the destination node.
         * @return true if the connection was removed, false otherwise.
         */
        bool RemoveConnection(NodeId from, NodeId to);

        /**
         * @brief Get all connections in the editor.
         * @return const std::vector<Connection>& Vector of connections.
         */
        const std::vector<Connection> &GetConnections() const;

        /**
         * @brief Remove all nodes and connections from the editor.
         */
        void Clear();

    private:
        std::unordered_map<NodeId, NodePtr> nodes; ///< Map of node IDs to node pointers
        std::vector<Connection> connections;       ///< List of connections
        NodeId nextId;                             ///< Next available node ID
    };

} // namespace VisionCraft::Engine
