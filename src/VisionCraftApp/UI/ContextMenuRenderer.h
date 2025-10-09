#pragma once

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Result of context menu rendering.
     */
    struct ContextMenuResult
    {
        enum class Action
        {
            None,
            DeleteNodes,
            CreateNode,
            CopyNodes,
            CutNodes,
            PasteNodes
        };

        Action action = Action::None;
        std::string nodeType; ///< For CreateNode action
    };

    /**
     * @brief Renders context menu for node editor.
     *
     * Separates context menu UI rendering from layer logic.
     * Supports delete operations and node creation via categorized submenu.
     */
    class ContextMenuRenderer
    {
    public:
        /**
         * @brief Structure representing a node type for creation menu.
         */
        struct NodeTypeInfo
        {
            std::string typeId;      ///< Internal type identifier
            std::string displayName; ///< User-facing display name
            std::string category;    ///< Category for grouping (e.g., "Input/Output", "Processing")
        };

        /**
         * @brief Renders the context menu and returns user action.
         * @param hasSelection Whether any nodes are selected
         * @param hasClipboardData Whether clipboard has data for paste
         * @return Result indicating what action was taken
         */
        [[nodiscard]] ContextMenuResult Render(bool hasSelection, bool hasClipboardData = false);

        /**
         * @brief Registers available node types for creation menu.
         * @param nodeTypes Vector of node type information
         */
        void SetAvailableNodeTypes(const std::vector<NodeTypeInfo> &nodeTypes);

        /**
         * @brief Opens the context menu popup.
         */
        void Open();

    private:
        std::vector<NodeTypeInfo> availableNodeTypes;

        /**
         * @brief Renders the "Add Node" submenu.
         * @return Node type ID if selected, empty string otherwise
         */
        [[nodiscard]] std::string RenderAddNodeSubmenu();
    };
} // namespace VisionCraft
