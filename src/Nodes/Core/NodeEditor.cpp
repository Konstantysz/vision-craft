#include "Nodes/Core/NodeEditor.h"
#include "Logger.h"
#include "Vision/Factory/NodeFactory.h"

#include <algorithm>
#include <fstream>
#include <queue>
#include <unordered_map>

namespace VisionCraft::Nodes
{

    NodeEditor::NodeEditor() : nextId(1)
    {
    }

    NodeId NodeEditor::AddNode(NodePtr node)
    {
        NodeId id = node->GetId();
        if (id >= nextId)
        {
            nextId = id + 1;
        }

        nodes[id] = std::move(node);

        return id;
    }

    bool NodeEditor::RemoveNode(NodeId id)
    {
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

        return true;
    }

    Node *NodeEditor::GetNode(NodeId id)
    {
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    const Node *NodeEditor::GetNode(NodeId id) const
    {
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    std::vector<NodeId> NodeEditor::GetNodeIds() const
    {
        std::vector<NodeId> ids;
        ids.reserve(nodes.size());
        for (const auto &[id, _] : nodes)
        {
            ids.push_back(id);
        }

        return ids;
    }

    void NodeEditor::AddConnection(NodeId from, NodeId to)
    {
        connections.push_back({ from, to });
    }

    bool NodeEditor::RemoveConnection(NodeId from, NodeId to)
    {
        auto it = std::remove_if(connections.begin(), connections.end(), [from, to](const Connection &c) {
            return c.from == from && c.to == to;
        });

        if (it == connections.end())
        {
            return false;
        }

        connections.erase(it, connections.end());

        return true;
    }

    const std::vector<Connection> &NodeEditor::GetConnections() const
    {
        return connections;
    }

    void NodeEditor::Clear()
    {
        nodes.clear();
        connections.clear();
        nextId = 1;
    }

    bool NodeEditor::Execute()
    {
        LOG_INFO("Executing graph with {} nodes", nodes.size());

        const auto executionOrder = TopologicalSort();
        if (executionOrder.empty() && !nodes.empty())
        {
            LOG_ERROR("Failed to determine execution order (cycle detected)");
            return false;
        }

        LOG_INFO("Execution order determined: {} nodes", executionOrder.size());

        for (const auto nodeId : executionOrder)
        {
            auto *node = GetNode(nodeId);
            if (!node)
                continue;

            try
            {
                for (const auto &conn : connections)
                {
                    if (conn.to == nodeId)
                    {
                        auto *fromNode = GetNode(conn.from);
                        PassDataBetweenNodes(fromNode, node);
                    }
                }

                LOG_INFO("Processing node: {} (ID: {})", node->GetName(), nodeId);
                node->Process();
            }
            catch (const std::exception &e)
            {
                LOG_ERROR("Node {} (ID: {}) failed during execution: {}", node->GetName(), nodeId, e.what());
                return false;
            }
            catch (...)
            {
                LOG_ERROR("Node {} (ID: {}) failed with unknown exception", node->GetName(), nodeId);
                return false;
            }
        }

        LOG_INFO("Graph execution completed successfully");
        return true;
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

    void NodeEditor::PassDataBetweenNodes(Node *fromNode, Node *toNode)
    {
        if (!fromNode || !toNode)
            return;

        if (!fromNode->HasOutputSlot("Output"))
        {
            LOG_WARN("Node {} has no output slot", fromNode->GetName());
            return;
        }

        const auto &outputSlot = fromNode->GetOutputSlot("Output");
        if (!outputSlot.HasData())
        {
            LOG_WARN("Node {} output slot has no data", fromNode->GetName());
            return;
        }

        if (!toNode->HasInputSlot("Input"))
        {
            LOG_WARN("Node {} has no input slot", toNode->GetName());
            return;
        }

        toNode->SetInputSlotData("Input", outputSlot.GetVariantData());
        LOG_INFO("Passed data from {} to {}", fromNode->GetName(), toNode->GetName());
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
                connJson["to"] = conn.to;
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
                for (const auto &nodeJson : j["nodes"])
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
                for (const auto &connJson : j["connections"])
                {
                    NodeId from = connJson["from"];
                    NodeId to = connJson["to"];
                    AddConnection(from, to);
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
