#include "ClipboardManager.h"

namespace VisionCraft
{
    void ClipboardManager::Copy(const std::unordered_set<Engine::NodeId> &selectedNodes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const std::vector<NodeConnection> &connections)
    {
        operation = ClipboardOperation::Copy;
        nodesToDelete.clear();
        DoCopy(selectedNodes, nodeTypes, nodeNames, nodePositions, connections);
    }

    void ClipboardManager::Cut(const std::unordered_set<Engine::NodeId> &selectedNodes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const std::vector<NodeConnection> &connections)
    {
        operation = ClipboardOperation::Cut;
        nodesToDelete.clear(); // Nodes are deleted immediately, no need to track
        DoCopy(selectedNodes, nodeTypes, nodeNames, nodePositions, connections);
    }

    void ClipboardManager::DoCopy(const std::unordered_set<Engine::NodeId> &selectedNodes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const std::vector<NodeConnection> &connections)
    {
        copiedNodes.clear();
        copiedConnections.clear();
        pasteCount = 0;

        // Copy node data
        for (const auto nodeId : selectedNodes)
        {
            CopiedNode copiedNode;
            copiedNode.originalId = nodeId;

            // Get node type
            if (nodeTypes.find(nodeId) != nodeTypes.end())
            {
                copiedNode.type = nodeTypes.at(nodeId);
            }

            // Get node name
            if (nodeNames.find(nodeId) != nodeNames.end())
            {
                copiedNode.name = nodeNames.at(nodeId);
            }

            // Get node position
            if (nodePositions.find(nodeId) != nodePositions.end())
            {
                copiedNode.position = nodePositions.at(nodeId);
            }

            copiedNodes.push_back(copiedNode);
        }

        // Copy connections (only internal connections between copied nodes)
        for (const auto &connection : connections)
        {
            const bool fromNodeCopied = selectedNodes.count(connection.outputPin.nodeId) > 0;
            const bool toNodeCopied = selectedNodes.count(connection.inputPin.nodeId) > 0;

            // Only copy connections where both nodes are in the selection
            if (fromNodeCopied && toNodeCopied)
            {
                CopiedConnection copiedConnection;
                copiedConnection.fromNodeId = connection.outputPin.nodeId;
                copiedConnection.toNodeId = connection.inputPin.nodeId;
                copiedConnection.fromSlot = connection.outputPin.pinName;
                copiedConnection.toSlot = connection.inputPin.pinName;
                copiedConnections.push_back(copiedConnection);
            }
        }
    }

    bool ClipboardManager::HasData() const
    {
        return !copiedNodes.empty();
    }

    ClipboardOperation ClipboardManager::GetOperation() const
    {
        return operation;
    }

    const std::vector<CopiedNode> &ClipboardManager::GetCopiedNodes() const
    {
        return copiedNodes;
    }

    const std::vector<CopiedConnection> &ClipboardManager::GetCopiedConnections() const
    {
        return copiedConnections;
    }

    const std::unordered_set<Engine::NodeId> &ClipboardManager::GetNodesToDelete() const
    {
        return nodesToDelete;
    }

    void ClipboardManager::Clear()
    {
        copiedNodes.clear();
        copiedConnections.clear();
        nodesToDelete.clear();
        operation = ClipboardOperation::None;
        pasteCount = 0;
    }

    ImVec2 ClipboardManager::GetPasteOffset() const
    {
        return ImVec2(PASTE_OFFSET_X * pasteCount, PASTE_OFFSET_Y * pasteCount);
    }

    void ClipboardManager::IncrementPasteCount()
    {
        ++pasteCount;
    }

    void ClipboardManager::ResetPasteCount()
    {
        pasteCount = 0;
    }

    void ClipboardManager::CompleteCutOperation()
    {
        nodesToDelete.clear();
        operation = ClipboardOperation::Copy; // Convert to copy after cut is complete
    }
} // namespace VisionCraft
