#pragma once

#include "UI/Widgets/NodeEditorTypes.h"

#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <imgui.h>

namespace VisionCraft::Editor::State
{
    /**
     * @brief Stores copied node data for clipboard operations.
     */
    struct CopiedNode
    {
        std::string type;                   ///< Nodes::Node type identifier
        std::string name;                   ///< Nodes::Node name
        UI::Widgets::NodePosition position; ///< Nodes::Node position
        Nodes::NodeId originalId;           ///< Original node ID (for connection remapping)
    };

    /**
     * @brief Stores copied connection data.
     */
    struct CopiedConnection
    {
        Nodes::NodeId fromNodeId; ///< Source node original ID
        Nodes::NodeId toNodeId;   ///< Destination node original ID
        std::string fromSlot;     ///< Source slot name
        std::string toSlot;       ///< Destination slot name
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
        void Copy(const std::unordered_set<Nodes::NodeId> &selectedNodes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const std::vector<UI::Widgets::NodeConnection> &connections);

        /**
         * @brief Cuts selected nodes to clipboard (copy + mark for deletion).
         * @param selectedNodes Set of node IDs to cut
         * @param nodeTypes Map of node ID to type string
         * @param nodeNames Map of node ID to name string
         * @param nodePositions Map of node ID to position
         * @param connections All connections in the graph
         */
        void Cut(const std::unordered_set<Nodes::NodeId> &selectedNodes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const std::vector<UI::Widgets::NodeConnection> &connections);

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
         * @brief Gets copied nodes as a read-only view.
         *
         * Uses C++20 std::span for a lightweight, non-owning view.
         *
         * @return Span of copied node data
         */
        [[nodiscard]] std::span<const CopiedNode> GetCopiedNodes() const;

        /**
         * @brief Gets copied connections (only internal to copied nodes) as a read-only view.
         *
         * Uses C++20 std::span for a lightweight, non-owning view.
         *
         * @return Span of copied connection data
         */
        [[nodiscard]] std::span<const CopiedConnection> GetCopiedConnections() const;

        /**
         * @brief Gets node IDs to delete (for Cut operation).
         * @return Set of node IDs that should be deleted after paste
         */
        [[nodiscard]] const std::unordered_set<Nodes::NodeId> &GetNodesToDelete() const;

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
        std::unordered_set<Nodes::NodeId> nodesToDelete; ///< For Cut operation
        ClipboardOperation operation = ClipboardOperation::None;
        int pasteCount = 0; ///< Number of times pasted (for offset calculation)

        static constexpr float PASTE_OFFSET_X = 50.0f;
        static constexpr float PASTE_OFFSET_Y = 50.0f;

        /**
         * @brief Internal copy implementation (used by both Copy and Cut).
         */
        void DoCopy(const std::unordered_set<Nodes::NodeId> &selectedNodes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeTypes,
            const std::unordered_map<Nodes::NodeId, std::string> &nodeNames,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const std::vector<UI::Widgets::NodeConnection> &connections);
    };
} // namespace VisionCraft::Editor::State
