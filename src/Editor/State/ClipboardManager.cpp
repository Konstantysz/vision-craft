#include "ClipboardManager.h"

namespace VisionCraft::Editor::State
{
    void ClipboardManager::Copy(const std::unordered_set<Nodes::NodeId> &selectedNodes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
        const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
        const std::vector<UI::Widgets::NodeConnection> &connections)
    {
        operation = ClipboardOperation::Copy;
        nodesToDelete.clear();
        DoCopy(selectedNodes, nodeTypes, nodeNames, nodePositions, connections);
    }

    void ClipboardManager::Cut(const std::unordered_set<Nodes::NodeId> &selectedNodes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
        const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
        const std::vector<UI::Widgets::NodeConnection> &connections)
    {
        operation = ClipboardOperation::Cut;
        nodesToDelete.clear(); // Nodes are deleted immediately, no need to track
        DoCopy(selectedNodes, nodeTypes, nodeNames, nodePositions, connections);
    }

    void ClipboardManager::DoCopy(const std::unordered_set<Nodes::NodeId> &selectedNodes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
        const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
        const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
        const std::vector<UI::Widgets::NodeConnection> &connections)
    {
        copiedNodes.clear();
        copiedConnections.clear();
        pasteCount = 0;

        // Copy node data - using C++20 designated initializers for clarity
        for (const auto nodeId : selectedNodes)
        {
            // Use designated initializers for better readability and explicit field assignment
            copiedNodes.push_back(CopiedNode{ .type = nodeTypes.contains(nodeId) ? nodeTypes.at(nodeId) : "",
                .name = nodeNames.contains(nodeId) ? nodeNames.at(nodeId) : "",
                .position = nodePositions.contains(nodeId) ? nodePositions.at(nodeId) : UI::Widgets::NodePosition{},
                .originalId = nodeId });
        }

        // Copy connections (only internal connections between copied nodes)
        for (const auto &connection : connections)
        {
            // C++20: Use contains() instead of count() for clearer intent
            const bool fromNodeCopied = selectedNodes.contains(connection.outputPin.nodeId);
            const bool toNodeCopied = selectedNodes.contains(connection.inputPin.nodeId);

            // Only copy connections where both nodes are in the selection
            if (fromNodeCopied && toNodeCopied)
            {
                // C++20 designated initializers for explicit field assignment
                copiedConnections.push_back(CopiedConnection{ .fromNodeId = connection.outputPin.nodeId,
                    .toNodeId = connection.inputPin.nodeId,
                    .fromSlot = connection.outputPin.pinName,
                    .toSlot = connection.inputPin.pinName });
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

    std::span<const CopiedNode> ClipboardManager::GetCopiedNodes() const
    {
        return copiedNodes;
    }

    std::span<const CopiedConnection> ClipboardManager::GetCopiedConnections() const
    {
        return copiedConnections;
    }

    const std::unordered_set<Nodes::NodeId> &ClipboardManager::GetNodesToDelete() const
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
        return ImVec2(PASTE_OFFSET_X * static_cast<float>(pasteCount), PASTE_OFFSET_Y * static_cast<float>(pasteCount));
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
} // namespace VisionCraft::Editor::State
