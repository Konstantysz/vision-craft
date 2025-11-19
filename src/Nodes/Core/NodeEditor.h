#pragma once
#include "Nodes/Core/Node.h"

#include <nlohmann/json.hpp>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <span>
#include <stop_token>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Nodes
{

    /**
     * @brief Callback for execution progress updates.
     * @param current Current node index (1-based)
     * @param total Total number of nodes
     * @param nodeName Name of the node being processed
     */
    using ExecutionProgressCallback = std::function<void(int current, int total, const std::string &nodeName)>;

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
         * @brief Returns all connections as a read-only view.
         *
         * Uses C++20 std::span for a lightweight, non-owning view of the connections.
         * This is more flexible than returning a const reference as it works with any
         * contiguous sequence.
         *
         * @return Span of connections
         */
        [[nodiscard]] std::span<const Connection> GetConnections() const;

        /**
         * @brief Removes all nodes and connections.
         */
        void Clear();

        /**
         * @brief Executes node graph in dependency order.
         * @param progressCallback Optional callback for progress updates
         * @param stopToken Token to check for cancellation requests
         * @return True if succeeded, false if cycle detected or cancelled
         * @note Performs topological sort, passes data between nodes, and calls Process() in order.
         */
        bool Execute(const ExecutionProgressCallback &progressCallback = nullptr, std::stop_token stopToken = {});

        /**
         * @brief Executes node graph asynchronously in background thread.
         * @param progressCallback Optional callback for progress updates
         * @return Future that will contain execution result
         * @note Use this to prevent UI blocking. Check future.valid() and future.wait_for() to poll status.
         */
        std::shared_future<bool> ExecuteAsync(const ExecutionProgressCallback &progressCallback = nullptr);

        /**
         * @brief Requests cancellation of current execution.
         * @note This is thread-safe and can be called from any thread.
         */
        void CancelExecution();

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

        mutable std::mutex graphMutex;             ///< Mutex for thread safety
        std::stop_source stopSource;               ///< Source for cancellation requests
        std::shared_future<bool> currentExecution; ///< Handle to current async execution
    };

} // namespace VisionCraft::Nodes
