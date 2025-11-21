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
     * @brief Type of connection between nodes (Blueprint-inspired).
     */
    enum class ConnectionType
    {
        Execution, ///< Execution flow connection (white wire) - defines execution order
        Data       ///< Data connection (colored wire) - transfers data between slots
    };

    /**
     * @brief Connection between two node slots.
     *
     * Supports both execution flow connections (Blueprint white wires) and data
     * connections (colored wires). Execution connections control which nodes execute
     * and in what order, while data connections transfer information between nodes.
     */
    struct Connection
    {
        NodeId from;                                ///< Source node ID
        std::string fromSlot;                       ///< Source slot name
        NodeId to;                                  ///< Destination node ID
        std::string toSlot;                         ///< Destination slot name
        ConnectionType type = ConnectionType::Data; ///< NEW: Connection type (execution or data)
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
         * @brief Destroy the NodeEditor object.
         * @note Ensures cancellation and wait for async execution to complete.
         */
        ~NodeEditor();

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
         * @brief Adds connection between node slots.
         * @param from Source node ID
         * @param fromSlot Source slot name
         * @param to Destination node ID
         * @param toSlot Destination slot name
         */
        void AddConnection(NodeId from, const std::string &fromSlot, NodeId to, const std::string &toSlot);

        /**
         * @brief Removes connection between node slots.
         * @param from Source node ID
         * @param fromSlot Source slot name
         * @param to Destination node ID
         * @param toSlot Destination slot name
         * @return True if removed
         */
        [[nodiscard]] bool
            RemoveConnection(NodeId from, const std::string &fromSlot, NodeId to, const std::string &toSlot);

        /**
         * @brief Returns a copy of all connections.
         *
         * Returns a vector copy instead of a span/reference to ensure thread safety.
         * The returned vector is a snapshot at the time of the call and won't be
         * invalidated by concurrent modifications to the graph.
         *
         * @return Vector containing copies of all connections
         */
        [[nodiscard]] std::vector<Connection> GetConnections() const;

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
         * @brief Execution step in cached execution plan (Blueprint-inspired bytecode foundation).
         *
         * Represents a single "instruction" in the execution plan, containing a node to execute
         * and the data connections that feed into it. This structure enables cache-friendly
         * linear execution without recomputing topological sort or searching connections.
         */
        struct ExecutionStep
        {
            NodeId nodeId;                               ///< Node to execute at this step
            std::vector<Connection> incomingConnections; ///< Data to pass before execution
        };

        /**
         * @brief Execution frame tracking current execution state (Blueprint FFrame equivalent).
         *
         * Manages the "instruction pointer" (current step index) for graph execution,
         * implementing Blueprint's signature lookahead advancement pattern where the
         * instruction pointer advances BEFORE node execution, not after.
         */
        struct ExecutionFrame
        {
            size_t instructionIndex = 0;                              ///< Current instruction pointer (step index)
            const ExecutionStep *currentStep = nullptr;               ///< Pointer to current step
            std::chrono::high_resolution_clock::time_point startTime; ///< Execution start time

            // Execution statistics for profiling
            struct Statistics
            {
                std::vector<std::chrono::microseconds> nodeExecutionTimes; ///< Per-node execution times
                size_t nodesExecuted = 0;                                  ///< Total nodes executed
                size_t dataPassOperations = 0;                             ///< Total data transfer operations
            } stats;

            /**
             * @brief Advances instruction pointer to next step (Blueprint lookahead pattern).
             *
             * This is the core of Blueprint's white execution wire system: the instruction
             * pointer advances BEFORE the node executes, not after. This "lookahead
             * advancement" is what enables efficient bytecode-style execution.
             *
             * @param plan Execution plan to advance through
             */
            void AdvanceToNext(const std::vector<ExecutionStep> &plan)
            {
                if (instructionIndex < plan.size())
                {
                    currentStep = &plan[instructionIndex];
                    ++instructionIndex; // Lookahead advancement - Blueprint pattern!
                }
                else
                {
                    currentStep = nullptr;
                }
            }

            /**
             * @brief Checks if execution is finished.
             * @param plan Execution plan being executed
             * @return True if all steps executed
             */
            [[nodiscard]] bool IsFinished(const std::vector<ExecutionStep> &plan) const
            {
                return instructionIndex >= plan.size();
            }

            /**
             * @brief Records execution time for a node (for profiling).
             * @param duration Time taken to execute node
             */
            void RecordNodeExecution(std::chrono::microseconds duration)
            {
                stats.nodeExecutionTimes.push_back(duration);
                stats.nodesExecuted++;
            }
        };

        /**
         * @brief Performs topological sort on node graph.
         * @return Node IDs in execution order, or empty if cycle detected
         */
        [[nodiscard]] std::vector<NodeId> TopologicalSort() const;

        /**
         * @brief Builds cached execution plan from current graph topology.
         *
         * Performs topological sort and precomputes incoming connections for each node,
         * enabling fast repeated execution without reanalysis. This is the "compilation"
         * phase in the Blueprint-inspired execution model.
         *
         * @return Vector of execution steps in dependency order
         */
        [[nodiscard]] std::vector<ExecutionStep> BuildExecutionPlan() const;

        /**
         * @brief Invalidates cached execution plan, forcing recompilation on next execute.
         *
         * Called automatically when graph structure changes (add/remove nodes/connections).
         * This implements the invalidation phase of the cache-aside pattern.
         */
        void InvalidateExecutionPlan();

        /**
         * @brief Passes data between nodes using slot system.
         * @param fromNode Source node
         * @param toNode Destination node
         * @param fromSlotName Source slot name
         * @param toSlotName Destination slot name
         */
        static void PassDataBetweenNodes(Node *fromNode,
            Node *toNode,
            const std::string &fromSlotName,
            const std::string &toSlotName);

        std::unordered_map<NodeId, NodePtr> nodes; ///< Node storage
        std::vector<Connection> connections;       ///< Connections
        NodeId nextId;                             ///< Next available ID

        mutable std::recursive_mutex graphMutex;                ///< Mutex for thread safety
        std::stop_source stopSource;                            ///< Source for cancellation requests
        std::shared_future<bool> currentExecution;              ///< Handle to current async execution
        mutable std::vector<ExecutionStep> cachedExecutionPlan; ///< Cached execution plan (mutable for lazy init)
        mutable bool executionPlanValid = false;                ///< Cache validity flag
    };

} // namespace VisionCraft::Nodes
