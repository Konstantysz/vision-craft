#pragma once

#include "NodeEditorTypes.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Stores copied node data for clipboard operations.
     */
    struct CopiedNode
    {
        std::string type;          ///< Node type identifier
        std::string name;          ///< Node name
        NodePosition position;     ///< Node position
        Engine::NodeId originalId; ///< Original node ID (for connection remapping)
    };

    /**
     * @brief Stores copied connection data.
     */
    struct CopiedConnection
    {
        Engine::NodeId fromNodeId; ///< Source node original ID
        Engine::NodeId toNodeId;   ///< Destination node original ID
        std::string fromSlot;      ///< Source slot name
        std::string toSlot;        ///< Destination slot name
    };

    /**
     * @brief Clipboard operation type.
     */
    enum class ClipboardOperation
    {
        None,
        Copy, ///< Copy operation (Ctrl+C)
        Cut   ///< Cut operation (Ctrl+X)
    };

    /**
     * @brief Manages copy/cut/paste operations for nodes.
     *
     * Handles copying and cutting selected nodes with their internal connections,
     * and pasting them with smart positioning and ID remapping.
     */
    class ClipboardManager
    {
    public:
        /**
         * @brief Copies selected nodes to clipboard.
         * @param selectedNodes Set of node IDs to copy
         * @param nodeTypes Map of node ID to type string
         * @param nodeNames Map of node ID to name string
         * @param nodePositions Map of node ID to position
         * @param connections All connections in the graph
         */
        void Copy(const std::unordered_set<Engine::NodeId> &selectedNodes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const std::vector<NodeConnection> &connections);

        /**
         * @brief Cuts selected nodes to clipboard (copy + mark for deletion).
         * @param selectedNodes Set of node IDs to cut
         * @param nodeTypes Map of node ID to type string
         * @param nodeNames Map of node ID to name string
         * @param nodePositions Map of node ID to position
         * @param connections All connections in the graph
         */
        void Cut(const std::unordered_set<Engine::NodeId> &selectedNodes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const std::vector<NodeConnection> &connections);

        /**
         * @brief Checks if clipboard has data.
         * @return True if clipboard contains copied/cut nodes
         */
        [[nodiscard]] bool HasData() const;

        /**
         * @brief Gets clipboard operation type.
         * @return Operation type (Copy, Cut, or None)
         */
        [[nodiscard]] ClipboardOperation GetOperation() const;

        /**
         * @brief Gets copied nodes.
         * @return Vector of copied node data
         */
        [[nodiscard]] const std::vector<CopiedNode> &GetCopiedNodes() const;

        /**
         * @brief Gets copied connections (only internal to copied nodes).
         * @return Vector of copied connection data
         */
        [[nodiscard]] const std::vector<CopiedConnection> &GetCopiedConnections() const;

        /**
         * @brief Gets node IDs to delete (for Cut operation).
         * @return Set of node IDs that should be deleted after paste
         */
        [[nodiscard]] const std::unordered_set<Engine::NodeId> &GetNodesToDelete() const;

        /**
         * @brief Clears clipboard.
         */
        void Clear();

        /**
         * @brief Calculates paste offset for smart positioning.
         * @return Offset to apply to pasted nodes
         */
        [[nodiscard]] ImVec2 GetPasteOffset() const;

        /**
         * @brief Increments paste count (for cascading pastes).
         */
        void IncrementPasteCount();

        /**
         * @brief Resets paste count.
         */
        void ResetPasteCount();

        /**
         * @brief Marks cut operation as completed (clears nodes to delete).
         */
        void CompleteCutOperation();

    private:
        std::vector<CopiedNode> copiedNodes;
        std::vector<CopiedConnection> copiedConnections;
        std::unordered_set<Engine::NodeId> nodesToDelete; ///< For Cut operation
        ClipboardOperation operation = ClipboardOperation::None;
        int pasteCount = 0; ///< Number of times pasted (for offset calculation)

        static constexpr float PASTE_OFFSET_X = 50.0f;
        static constexpr float PASTE_OFFSET_Y = 50.0f;

        /**
         * @brief Internal copy implementation (used by both Copy and Cut).
         */
        void DoCopy(const std::unordered_set<Engine::NodeId> &selectedNodes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Engine::NodeId, std::string> &nodeNames,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const std::vector<NodeConnection> &connections);
    };
} // namespace VisionCraft