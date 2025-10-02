#include "NodeEditor.h"
#include "Logger.h"

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/PreviewNode.h"
#include "Nodes/ThresholdNode.h"

#include <algorithm>
#include <queue>
#include <unordered_map>

namespace VisionCraft::Engine
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

        // Get execution order using topological sort
        const auto executionOrder = TopologicalSort();
        if (executionOrder.empty() && !nodes.empty())
        {
            LOG_ERROR("Failed to determine execution order (cycle detected)");
            return false;
        }

        LOG_INFO("Execution order determined: {} nodes", executionOrder.size());

        // Execute nodes in topological order
        for (const auto nodeId : executionOrder)
        {
            auto *node = GetNode(nodeId);
            if (!node)
                continue;

            try
            {
                // First, pass data from all incoming connections
                for (const auto &conn : connections)
                {
                    if (conn.to == nodeId)
                    {
                        auto *fromNode = GetNode(conn.from);
                        PassDataBetweenNodes(fromNode, node);
                    }
                }

                // Then execute the node
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

        // Build adjacency list and in-degree map
        std::unordered_map<NodeId, std::vector<NodeId>> adjList;
        std::unordered_map<NodeId, int> inDegree;

        // Initialize all nodes with 0 in-degree
        for (const auto nodeId : nodeIds)
        {
            inDegree[nodeId] = 0;
            adjList[nodeId] = {};
        }

        // Build graph from connections
        for (const auto &conn : connections)
        {
            adjList[conn.from].push_back(conn.to);
            inDegree[conn.to]++;
        }

        // Kahn's algorithm for topological sort
        std::queue<NodeId> queue;
        std::vector<NodeId> executionOrder;

        // Add all nodes with in-degree 0 to queue
        for (const auto nodeId : nodeIds)
        {
            if (inDegree[nodeId] == 0)
            {
                queue.push(nodeId);
            }
        }

        // Process nodes
        while (!queue.empty())
        {
            const auto currentNode = queue.front();
            queue.pop();
            executionOrder.push_back(currentNode);

            // Decrease in-degree of neighbors
            for (const auto neighbor : adjList[currentNode])
            {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0)
                {
                    queue.push(neighbor);
                }
            }
        }

        // Check for cycles
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

        cv::Mat outputImage;
        bool hasOutput = false;

        // Extract output image from source node (using dynamic_cast for type checking)
        // TODO: Replace this with a proper polymorphic interface or data port system
        if (auto *inputNode = dynamic_cast<ImageInputNode *>(fromNode))
        {
            outputImage = inputNode->GetOutputImage();
            hasOutput = !outputImage.empty();
        }
        else if (auto *grayscaleNode = dynamic_cast<GrayscaleNode *>(fromNode))
        {
            outputImage = grayscaleNode->GetOutputImage();
            hasOutput = grayscaleNode->HasValidOutput();
        }
        else if (auto *cannyNode = dynamic_cast<CannyEdgeNode *>(fromNode))
        {
            outputImage = cannyNode->GetOutputImage();
            hasOutput = !outputImage.empty();
        }
        else if (auto *thresholdNode = dynamic_cast<ThresholdNode *>(fromNode))
        {
            outputImage = thresholdNode->GetOutputImage();
            hasOutput = !outputImage.empty();
        }

        if (!hasOutput)
        {
            LOG_WARN("Node {} has no output image", fromNode->GetName());
            return;
        }

        // Pass data to destination node (using dynamic_cast for type checking)
        bool dataPassed = false;

        if (auto *previewNode = dynamic_cast<PreviewNode *>(toNode))
        {
            previewNode->SetInputImage(outputImage);
            dataPassed = true;
        }
        else if (auto *grayscaleNode = dynamic_cast<GrayscaleNode *>(toNode))
        {
            grayscaleNode->SetInputImage(outputImage);
            dataPassed = true;
        }
        else if (auto *cannyNode = dynamic_cast<CannyEdgeNode *>(toNode))
        {
            cannyNode->SetInputImage(outputImage);
            dataPassed = true;
        }
        else if (auto *thresholdNode = dynamic_cast<ThresholdNode *>(toNode))
        {
            thresholdNode->SetInputImage(outputImage);
            dataPassed = true;
        }
        else if (auto *outputNode = dynamic_cast<ImageOutputNode *>(toNode))
        {
            outputNode->SetInputImage(outputImage);
            dataPassed = true;
        }

        if (dataPassed)
        {
            LOG_INFO("Passing image from {} to {}", fromNode->GetName(), toNode->GetName());
        }
    }

} // namespace VisionCraft::Engine
