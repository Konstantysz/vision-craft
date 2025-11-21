#include "Nodes/Core/NodeEditor.h"
#include "Logger.h"
#include "Vision/Factory/NodeFactory.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace VisionCraft::Nodes
{

    NodeEditor::NodeEditor() : nextId(1)
    {
    }

    NodeEditor::~NodeEditor()
    {
        CancelExecution();
        if (currentExecution.valid())
        {
            currentExecution.wait();
        }
    }

    NodeId NodeEditor::AddNode(NodePtr node)
    {
        std::scoped_lock lock(graphMutex);
        NodeId id = node->GetId();
        if (id >= nextId)
        {
            nextId = id + 1;
        }

        nodes[id] = std::move(node);
        InvalidateExecutionPlan(); // Graph structure changed

        return id;
    }

    bool NodeEditor::RemoveNode(NodeId id)
    {
        std::scoped_lock lock(graphMutex);
        auto it = nodes.find(id);
        if (it == nodes.end())
        {
            return false;
        }

        nodes.erase(it);
        connections.erase(std::remove_if(connections.begin(),
                              connections.end(),
                              [id](const Connection &c) { return c.from == id || c.to == id; }),
            connections.end());

        InvalidateExecutionPlan(); // Graph structure changed
        return true;
    }

    Node *NodeEditor::GetNode(NodeId id)
    {
        std::scoped_lock lock(graphMutex);
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    const Node *NodeEditor::GetNode(NodeId id) const
    {
        std::scoped_lock lock(graphMutex);
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    std::vector<NodeId> NodeEditor::GetNodeIds() const
    {
        std::scoped_lock lock(graphMutex);
        // C++20 ranges: Extract keys from map using views::keys
        auto keys = nodes | std::views::keys;
        return std::vector<NodeId>(keys.begin(), keys.end());
    }

    void NodeEditor::AddConnection(NodeId from,
        const std::string &fromSlot,
        NodeId to,
        const std::string &toSlot,
        ConnectionType type)
    {
        std::scoped_lock lock(graphMutex);
        // C++20 designated initializers for clarity
        connections.push_back({ .from = from, .fromSlot = fromSlot, .to = to, .toSlot = toSlot, .type = type });
        InvalidateExecutionPlan(); // Graph structure changed
    }

    bool NodeEditor::RemoveConnection(NodeId from, const std::string &fromSlot, NodeId to, const std::string &toSlot)
    {
        std::scoped_lock lock(graphMutex);
        auto it = std::remove_if(connections.begin(), connections.end(), [&](const Connection &c) {
            return c.from == from && c.fromSlot == fromSlot && c.to == to && c.toSlot == toSlot;
        });

        if (it == connections.end())
        {
            return false;
        }

        connections.erase(it, connections.end());
        InvalidateExecutionPlan(); // Graph structure changed

        return true;
    }

    std::vector<Connection> NodeEditor::GetConnections() const
    {
        std::scoped_lock lock(graphMutex);
        return connections; // Returns a copy for thread safety
    }

    void NodeEditor::Clear()
    {
        std::scoped_lock lock(graphMutex);
        nodes.clear();
        connections.clear();
        nextId = 1;
        InvalidateExecutionPlan(); // Graph structure changed
    }

    bool NodeEditor::Execute(const ExecutionProgressCallback &progressCallback, std::stop_token stopToken)
    {
        std::unique_lock lock(graphMutex);

        // If running synchronously (no external token), reset stopSource to allow fresh cancellation
        if (!stopToken.stop_possible())
        {
            stopSource = std::stop_source();
        }

        LOG_INFO("Executing graph with {} nodes", nodes.size());

        // Use cached execution plan (compilation phase)
        if (!executionPlanValid)
        {
            cachedExecutionPlan = BuildExecutionPlan();
            if (cachedExecutionPlan.empty() && !nodes.empty())
            {
                LOG_ERROR("Failed to build execution plan (cycle detected)");
                return false;
            }
            executionPlanValid = true;
        }

        LOG_INFO("Executing {} steps from cached plan", cachedExecutionPlan.size());

        // Create execution frame
        ExecutionFrame frame;
        frame.startTime = std::chrono::high_resolution_clock::now();
        int totalNodes = static_cast<int>(cachedExecutionPlan.size());

        // Execute using frame with lookahead advancement
        while (!frame.IsFinished(cachedExecutionPlan))
        {
            if (stopToken.stop_requested() || stopSource.stop_requested())
            {
                LOG_WARN("Graph execution cancelled by user");
                return false;
            }

            // LOOKAHEAD ADVANCEMENT: Advance instruction pointer BEFORE execution
            frame.AdvanceToNext(cachedExecutionPlan);

            const auto &step = *frame.currentStep;

            // Direct node lookup (no function call overhead)
            auto it = nodes.find(step.nodeId);
            if (it == nodes.end())
                continue;
            Node *node = it->second.get();

            if (progressCallback)
            {
                progressCallback(static_cast<int>(frame.instructionIndex), totalNodes, node->GetName());
            }

            try
            {
                // Use precomputed incoming connections (no search overhead)
                for (const auto &conn : step.incomingConnections)
                {
                    auto fromIt = nodes.find(conn.from);
                    if (fromIt != nodes.end())
                    {
                        PassDataBetweenNodes(fromIt->second.get(), node, conn.fromSlot, conn.toSlot);
                        frame.stats.dataPassOperations++;
                    }
                }

                LOG_INFO("Processing node: {} (ID: {})", node->GetName(), step.nodeId);

                // Time the node execution for profiling
                auto nodeStartTime = std::chrono::high_resolution_clock::now();
                node->Process();
                auto nodeEndTime = std::chrono::high_resolution_clock::now();

                auto nodeDuration = std::chrono::duration_cast<std::chrono::microseconds>(nodeEndTime - nodeStartTime);
                frame.RecordNodeExecution(nodeDuration);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR("Node {} (ID: {}) failed during execution: {}", node->GetName(), step.nodeId, e.what());
                return false;
            }
            catch (...)
            {
                LOG_ERROR("Node {} (ID: {}) failed with unknown exception", node->GetName(), step.nodeId);
                return false;
            }
        }

        LOG_INFO("Graph execution completed successfully");
        return true;
    }

    std::shared_future<bool> NodeEditor::ExecuteAsync(const ExecutionProgressCallback &progressCallback)
    {
        // Create a new stop source for this execution
        stopSource = std::stop_source();
        std::stop_token stopToken = stopSource.get_token();

        // Launch async task and store in shared_future
        // Note: std::async returns std::future, which is movable to std::shared_future
        currentExecution = std::async(
            std::launch::async, [this, progressCallback, stopToken]() { return Execute(progressCallback, stopToken); });

        return currentExecution;
    }

    void NodeEditor::CancelExecution()
    {
        stopSource.request_stop();
    }

    std::vector<NodeId> NodeEditor::TopologicalSort() const
    {
        const auto nodeIds = GetNodeIds();

        std::unordered_map<NodeId, std::vector<NodeId>> adjList;
        std::unordered_map<NodeId, int> inDegree;

        for (const auto nodeId : nodeIds)
        {
            inDegree[nodeId] = 0;
            adjList[nodeId] = {};
        }

        for (const auto &conn : connections)
        {
            adjList[conn.from].push_back(conn.to);
            inDegree[conn.to]++;
        }

        // Kahn's algorithm for topological sort
        std::queue<NodeId> queue;
        std::vector<NodeId> executionOrder;

        for (const auto nodeId : nodeIds)
        {
            if (inDegree[nodeId] == 0)
            {
                queue.push(nodeId);
            }
        }

        while (!queue.empty())
        {
            const auto currentNode = queue.front();
            queue.pop();
            executionOrder.push_back(currentNode);

            for (const auto neighbor : adjList[currentNode])
            {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0)
                {
                    queue.push(neighbor);
                }
            }
        }

        if (executionOrder.size() != nodeIds.size())
        {
            LOG_ERROR("Cycle detected in node graph! Cannot execute.");
            return {};
        }

        return executionOrder;
    }

    std::vector<NodeEditor::ExecutionStep> NodeEditor::BuildExecutionPlan() const
    {
        LOG_INFO("Building execution plan (compilation phase - execution flow)");

        const auto nodeIds = GetNodeIds();
        if (nodeIds.empty())
        {
            return {};
        }

        // Separate execution and data connections
        std::vector<Connection> executionConnections;
        std::vector<Connection> dataConnections;

        for (const auto &conn : connections)
        {
            if (conn.type == ConnectionType::Execution)
            {
                executionConnections.push_back(conn);
                LOG_DEBUG("Execution connection: {} ({}) -> {} ({})", conn.from, conn.fromSlot, conn.to, conn.toSlot);
            }
            else
            {
                dataConnections.push_back(conn);
                LOG_DEBUG("Data connection: {} ({}) -> {} ({})", conn.from, conn.fromSlot, conn.to, conn.toSlot);
            }
        }

        LOG_INFO("Total connections: {} execution, {} data", executionConnections.size(), dataConnections.size());

        // Determine which nodes have execution pins
        std::unordered_set<NodeId> nodesWithExecutionPins;
        for (const auto nodeId : nodeIds)
        {
            const auto *node = GetNode(nodeId);
            if (node && (!node->GetExecutionInputPins().empty() || !node->GetExecutionOutputPins().empty()))
            {
                nodesWithExecutionPins.insert(nodeId);
                LOG_DEBUG("Node {} (type: {}) has execution pins", nodeId, node->GetType());
            }
        }

        LOG_INFO("Nodes with execution pins: {}", nodesWithExecutionPins.size());

        std::vector<NodeId> executionOrder;

        // If there are execution connections, follow execution flow
        if (!executionConnections.empty())
        {
            LOG_INFO("Building execution order from {} execution flow connections", executionConnections.size());

            // Build adjacency list for execution connections only
            std::unordered_map<NodeId, std::vector<NodeId>> execAdjList;
            std::unordered_map<NodeId, int> execInDegree;

            for (const auto nodeId : nodeIds)
            {
                execInDegree[nodeId] = 0;
                execAdjList[nodeId] = {};
            }

            for (const auto &conn : executionConnections)
            {
                execAdjList[conn.from].push_back(conn.to);
                execInDegree[conn.to]++;
            }

            // Find entry points: nodes with execution outputs but no execution inputs
            // Entry points are starter nodes (e.g., ImageInput with only output execution pin)
            std::queue<NodeId> queue;
            for (const auto nodeId : nodeIds)
            {
                const auto *node = GetNode(nodeId);
                if (!node)
                    continue;

                // Entry point must have:
                // 1. Execution output pin(s)
                // 2. NO execution input pin (it's a starter)
                const bool hasExecutionOutput = !node->GetExecutionOutputPins().empty();
                const bool hasExecutionInput = !node->GetExecutionInputPins().empty();

                if (hasExecutionOutput && !hasExecutionInput)
                {
                    queue.push(nodeId);
                    LOG_DEBUG("Entry point node found: {} (type: {})", nodeId, node->GetType());
                }
            }

            // Traverse execution flow graph (simple linear traversal, 1:1 connections prevent cycles)
            while (!queue.empty())
            {
                const auto currentNode = queue.front();
                queue.pop();
                executionOrder.push_back(currentNode);

                for (const auto neighbor : execAdjList[currentNode])
                {
                    execInDegree[neighbor]--;
                    if (execInDegree[neighbor] == 0)
                    {
                        queue.push(neighbor);
                    }
                }
            }

            // Verify all nodes with execution pins are in execution order
            std::unordered_set<NodeId> nodesInExecutionOrder(executionOrder.begin(), executionOrder.end());

            for (const auto nodeId : nodesWithExecutionPins)
            {
                if (nodesInExecutionOrder.count(nodeId) == 0)
                {
                    const auto *node = GetNode(nodeId);
                    LOG_ERROR(
                        "Node {} (type: {}) has execution pins but is not connected to execution flow. "
                        "Please connect its execution pins (white arrows).",
                        nodeId,
                        node ? node->GetType() : "unknown");
                    return {};
                }
            }

            LOG_INFO("Execution flow order: {} nodes", executionOrder.size());
        }
        else
        {
            // No execution connections - check if any nodes have execution pins
            if (!nodesWithExecutionPins.empty())
            {
                LOG_ERROR(
                    "Graph contains {} nodes with execution pins but no execution connections. "
                    "Please connect execution pins (white arrows) to define execution flow.",
                    nodesWithExecutionPins.size());
                return {};
            }

            // No execution pins at all - fall back to pure data dependency order (legacy graphs)
            LOG_INFO("No execution connections found, using data dependency order (legacy mode)");
            executionOrder = TopologicalSort();
            if (executionOrder.empty() && !nodes.empty())
            {
                LOG_ERROR("Failed to build execution plan (cycle detected in data dependencies)");
                return {};
            }
        }

        // Build execution steps with precomputed incoming data connections
        std::vector<ExecutionStep> plan;
        plan.reserve(executionOrder.size());

        for (const auto nodeId : executionOrder)
        {
            ExecutionStep step;
            step.nodeId = nodeId;

            // Precompute all incoming DATA connections for this node
            // (execution connections control flow, data connections pass parameters)
            for (const auto &conn : dataConnections)
            {
                if (conn.to == nodeId)
                {
                    step.incomingConnections.push_back(conn);
                }
            }

            plan.push_back(std::move(step));
        }

        LOG_INFO("Execution plan compiled: {} steps", plan.size());
        return plan;
    }

    void NodeEditor::InvalidateExecutionPlan()
    {
        executionPlanValid = false;
        LOG_DEBUG("Execution plan invalidated (graph structure changed)");
    }

    void NodeEditor::PassDataBetweenNodes(Node *fromNode,
        Node *toNode,
        const std::string &fromSlotName,
        const std::string &toSlotName)
    {
        if (!fromNode || !toNode)
            return;

        if (!fromNode->HasOutputSlot(fromSlotName))
        {
            LOG_WARN("Node {} has no output slot '{}'", fromNode->GetName(), fromSlotName);
            return;
        }

        const auto &outputSlot = fromNode->GetOutputSlot(fromSlotName);
        if (!outputSlot.HasData())
        {
            LOG_WARN("Node {} output slot '{}' has no data", fromNode->GetName(), fromSlotName);
            return;
        }

        if (!toNode->HasInputSlot(toSlotName))
        {
            LOG_WARN("Node {} has no input slot '{}'", toNode->GetName(), toSlotName);
            return;
        }

        toNode->SetInputSlotData(toSlotName, outputSlot.GetVariantData());
        LOG_INFO(
            "Passed data from {} ({}) to {} ({})", fromNode->GetName(), fromSlotName, toNode->GetName(), toSlotName);
    }

    bool NodeEditor::SaveToFile(const std::filesystem::path &filepath,
        const std::unordered_map<NodeId, std::pair<float, float>> &nodePositions) const
    {
        try
        {
            nlohmann::json j;
            j["version"] = "1.0";

            // Serialize nodes
            nlohmann::json nodesArray = nlohmann::json::array();
            for (const auto &[id, nodePtr] : nodes)
            {
                if (!nodePtr)
                {
                    LOG_WARN("Skipping null node with ID: {}", id);
                    continue;
                }

                nlohmann::json nodeJson;
                nodeJson["id"] = id;
                nodeJson["type"] = nodePtr->GetType();
                nodeJson["name"] = nodePtr->GetName();
                nodesArray.push_back(nodeJson);
            }
            j["nodes"] = nodesArray;

            // Serialize connections
            nlohmann::json connectionsArray = nlohmann::json::array();
            for (const auto &conn : connections)
            {
                nlohmann::json connJson;
                connJson["from"] = conn.from;
                connJson["fromSlot"] = conn.fromSlot;
                connJson["to"] = conn.to;
                connJson["toSlot"] = conn.toSlot;
                connectionsArray.push_back(connJson);
            }
            j["connections"] = connectionsArray;

            // Serialize node positions
            nlohmann::json positionsArray = nlohmann::json::array();
            for (const auto &[id, pos] : nodePositions)
            {
                nlohmann::json posJson;
                posJson["id"] = id;
                posJson["x"] = pos.first;
                posJson["y"] = pos.second;
                positionsArray.push_back(posJson);
            }
            j["nodePositions"] = positionsArray;

            // Write to file
            std::ofstream file(filepath);
            if (!file.is_open())
            {
                LOG_ERROR("Failed to open file for writing: {}", filepath.string());
                return false;
            }

            file << j.dump(2);
            file.close();

            LOG_INFO("Saved graph to: {}", filepath.string());
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to save graph: {}", e.what());
            return false;
        }
    }

    bool NodeEditor::LoadFromFile(const std::filesystem::path &filepath,
        std::unordered_map<NodeId, std::pair<float, float>> &nodePositions)
    {
        try
        {
            std::ifstream file(filepath);
            if (!file.is_open())
            {
                LOG_ERROR("Failed to open file for reading: {}", filepath.string());
                return false;
            }

            nlohmann::json j;
            file >> j;
            file.close();

            // Clear existing graph
            Clear();
            nodePositions.clear();

            // Deserialize nodes
            if (j.contains("nodes"))
            {
                const auto &nodesArray = j["nodes"];

                // Impose limit on number of nodes to prevent DoS via memory exhaustion
                constexpr size_t kMaxNodes = 10000;
                if (nodesArray.size() > kMaxNodes)
                {
                    LOG_ERROR("Graph contains {} nodes, exceeding maximum of {}. File may be corrupted or malicious.",
                        nodesArray.size(),
                        kMaxNodes);
                    return false;
                }

                for (const auto &nodeJson : nodesArray)
                {
                    NodeId id = nodeJson["id"];
                    std::string type = nodeJson["type"];
                    std::string name = nodeJson["name"];

                    // Validate node type is registered
                    if (!Vision::NodeFactory::IsRegistered(type))
                    {
                        LOG_ERROR("Attempted to load unregistered node type: {} - skipping", type);
                        continue;
                    }

                    // Validate node ID is reasonable
                    constexpr NodeId kMaxNodeId = 1000000;
                    if (id < 0 || id > kMaxNodeId)
                    {
                        LOG_ERROR("Invalid node ID: {} - skipping", id);
                        continue;
                    }

                    // Validate name length to prevent excessive memory usage
                    constexpr size_t kMaxNameLength = 256;
                    if (name.length() > kMaxNameLength)
                    {
                        LOG_ERROR("Node name too long ({} chars), max is {} - skipping", name.length(), kMaxNameLength);
                        continue;
                    }

                    auto node = Vision::NodeFactory::CreateNode(type, id, name);
                    if (node)
                    {
                        AddNode(std::move(node));
                    }
                    else
                    {
                        LOG_ERROR("Failed to create node of type: {}", type);
                    }
                }
            }

            // Deserialize connections
            if (j.contains("connections"))
            {
                const auto &connectionsArray = j["connections"];

                // Impose limit on number of connections to prevent DoS
                constexpr size_t kMaxConnections = 50000;
                if (connectionsArray.size() > kMaxConnections)
                {
                    LOG_ERROR(
                        "Graph contains {} connections, exceeding maximum of {}. File may be corrupted or malicious.",
                        connectionsArray.size(),
                        kMaxConnections);
                    return false;
                }

                for (const auto &connJson : connectionsArray)
                {
                    NodeId from = connJson["from"];
                    NodeId to = connJson["to"];

                    // Support both old and new formats
                    std::string fromSlot =
                        connJson.contains("fromSlot") ? connJson["fromSlot"].get<std::string>() : "Output";
                    std::string toSlot = connJson.contains("toSlot") ? connJson["toSlot"].get<std::string>() : "Input";

                    // Validate slot name lengths
                    constexpr size_t kMaxSlotNameLength = 128;
                    if (fromSlot.length() > kMaxSlotNameLength || toSlot.length() > kMaxSlotNameLength)
                    {
                        LOG_WARN("Skipping connection with excessively long slot names");
                        continue;
                    }

                    // Validate that both nodes exist before adding connection
                    if (nodes.find(from) != nodes.end() && nodes.find(to) != nodes.end())
                    {
                        AddConnection(from, fromSlot, to, toSlot);
                    }
                    else
                    {
                        LOG_WARN("Skipping connection from {} to {} - node(s) not found", from, to);
                    }
                }
            }

            // Deserialize node positions
            if (j.contains("nodePositions"))
            {
                for (const auto &posJson : j["nodePositions"])
                {
                    NodeId id = posJson["id"];
                    float x = posJson["x"];
                    float y = posJson["y"];

                    // Validate position values (reject NaN, Inf, and unreasonable coordinates)
                    if (!std::isfinite(x) || !std::isfinite(y))
                    {
                        LOG_WARN("Skipping node {} with invalid position ({}, {})", id, x, y);
                        continue;
                    }

                    // Reject extremely large coordinates (likely corrupted data)
                    constexpr float kMaxCoordinate = 1000000.0f;
                    if (std::abs(x) > kMaxCoordinate || std::abs(y) > kMaxCoordinate)
                    {
                        LOG_WARN("Skipping node {} with unreasonable position ({}, {})", id, x, y);
                        continue;
                    }

                    nodePositions[id] = { x, y };
                }
            }

            LOG_INFO("Loaded graph from: {}", filepath.string());
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to load graph: {}", e.what());
            return false;
        }
    }

} // namespace VisionCraft::Nodes
