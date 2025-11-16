#pragma once

#include <functional>
#include <string>
#include <vector>

namespace VisionCraft::UI::Widgets
{
    /**
     * @brief Information about a node type for search palette.
     */
    struct SearchableNodeInfo
    {
        std::string typeId;      ///< Factory type ID (e.g., "Grayscale")
        std::string displayName; ///< User-friendly name (e.g., "Grayscale")
        std::string category;    ///< Category (e.g., "Processing")
        int useCount = 0;        ///< Number of times this node type has been used
        float matchScore = 0.0f; ///< Fuzzy search match score (0.0 = no match, 1.0 = perfect match)
    };

    /**
     * @brief Searchable node creation palette widget.
     *
     * Provides a fuzzy-searchable dialog for quick node creation,
     * similar to Blender's Shift+A or Unreal Engine's right-click search.
     * Tracks recently used nodes and displays them first in search results.
     */
    class NodeSearchPalette
    {
    public:
        /**
         * @brief Constructor.
         */
        NodeSearchPalette();

        /**
         * @brief Opens the search palette at the specified screen position.
         * @param screenPosX X coordinate in screen space
         * @param screenPosY Y coordinate in screen space
         */
        void Open(float screenPosX, float screenPosY);

        /**
         * @brief Closes the search palette.
         */
        void Close();

        /**
         * @brief Checks if the palette is currently open.
         * @return True if open, false otherwise
         */
        [[nodiscard]] bool IsOpen() const
        {
            return m_isOpen;
        }

        /**
         * @brief Renders the search palette (call every frame).
         * @return Selected node type ID, or empty string if none selected
         */
        [[nodiscard]] std::string Render();

        /**
         * @brief Sets the available node types for search.
         * @param nodeTypes Vector of searchable node information
         */
        void SetAvailableNodeTypes(const std::vector<SearchableNodeInfo> &nodeTypes);

        /**
         * @brief Records usage of a node type (for recent nodes tracking).
         * @param typeId Factory type ID of the created node
         */
        void RecordNodeUsage(const std::string &typeId);

        /**
         * @brief Gets the screen position where the palette was opened.
         * @param outX Output X coordinate
         * @param outY Output Y coordinate
         */
        void GetOpenPosition(float &outX, float &outY) const;

    private:
        /**
         * @brief Performs fuzzy search on node types.
         */
        void UpdateSearchResults();

        /**
         * @brief Calculates fuzzy match score between query and target string.
         * @param query Search query (lowercase)
         * @param target Target string to match against (will be converted to lowercase)
         * @return Match score (0.0 = no match, 1.0 = perfect match)
         */
        [[nodiscard]] static float CalculateFuzzyScore(const std::string &query, const std::string &target);

        /**
         * @brief Sorts search results by match score and usage count.
         */
        void SortSearchResults();

        bool m_isOpen = false;                            ///< Whether the palette is currently open
        float m_openPosX = 0.0f;                          ///< Screen X position where palette was opened
        float m_openPosY = 0.0f;                          ///< Screen Y position where palette was opened
        char m_searchBuffer[256] = { 0 };                 ///< Input buffer for search query
        std::vector<SearchableNodeInfo> m_availableNodes; ///< All available node types
        std::vector<SearchableNodeInfo> m_filteredNodes;  ///< Filtered search results
        int m_selectedIndex = 0;                          ///< Currently selected index in filtered results
        bool m_focusSearchInput = false;                  ///< Flag to focus search input next frame
    };
} // namespace VisionCraft::UI::Widgets
