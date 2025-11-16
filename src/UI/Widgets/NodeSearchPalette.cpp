#include "NodeSearchPalette.h"

#include <algorithm>
#include <cctype>

#include <imgui.h>

namespace VisionCraft::UI::Widgets
{
    NodeSearchPalette::NodeSearchPalette() = default;

    void NodeSearchPalette::Open(float screenPosX, float screenPosY)
    {
        m_isOpen = true;
        m_openPosX = screenPosX;
        m_openPosY = screenPosY;
        m_searchBuffer[0] = '\0';
        m_selectedIndex = 0;
        m_focusSearchInput = true;

        // Initialize filtered results with all nodes
        UpdateSearchResults();
    }

    void NodeSearchPalette::Close()
    {
        m_isOpen = false;
        m_searchBuffer[0] = '\0';
        m_selectedIndex = 0;
    }

    std::string NodeSearchPalette::Render()
    {
        if (!m_isOpen)
            return "";

        std::string selectedNodeType;

        // Set popup position
        ImGui::SetNextWindowPos(ImVec2(m_openPosX, m_openPosY), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

        if (ImGui::BeginPopup("##NodeSearchPalette", ImGuiWindowFlags_NoMove))
        {
            // Search input
            if (m_focusSearchInput)
            {
                ImGui::SetKeyboardFocusHere();
                m_focusSearchInput = false;
            }

            bool searchChanged =
                ImGui::InputText("##Search", m_searchBuffer, sizeof(m_searchBuffer), ImGuiInputTextFlags_AutoSelectAll);

            if (searchChanged)
            {
                UpdateSearchResults();
                m_selectedIndex = 0; // Reset selection when search changes
            }

            // Handle keyboard navigation
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_filteredNodes.size());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_filteredNodes.size()))
                                  % static_cast<int>(m_filteredNodes.size());
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Enter) && !m_filteredNodes.empty())
            {
                selectedNodeType = m_filteredNodes[m_selectedIndex].typeId;
                RecordNodeUsage(selectedNodeType);
                Close();
                ImGui::CloseCurrentPopup();
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                Close();
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            // Results list
            ImGui::BeginChild("##SearchResults", ImVec2(0, 0), false);

            if (m_filteredNodes.empty())
            {
                ImGui::TextDisabled("No nodes found");
            }
            else
            {
                for (int i = 0; i < static_cast<int>(m_filteredNodes.size()); ++i)
                {
                    const auto &node = m_filteredNodes[i];
                    bool isSelected = (i == m_selectedIndex);

                    // Format: "Display Name (Category)" or just "Display Name" if no category
                    std::string label = node.displayName;
                    if (!node.category.empty())
                    {
                        label += " (" + node.category + ")";
                    }

                    // Highlight recently used nodes
                    if (node.useCount > 0)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f)); // Light blue
                    }

                    if (ImGui::Selectable(label.c_str(), isSelected))
                    {
                        selectedNodeType = node.typeId;
                        RecordNodeUsage(selectedNodeType);
                        Close();
                        ImGui::CloseCurrentPopup();
                    }

                    if (node.useCount > 0)
                    {
                        ImGui::PopStyleColor();
                    }

                    // Scroll to selected item
                    if (isSelected && ImGui::IsItemVisible())
                    {
                        ImGui::SetScrollHereY(0.5f);
                    }
                }
            }

            ImGui::EndChild();
            ImGui::EndPopup();
        }
        else
        {
            // Popup was closed externally
            Close();
        }

        return selectedNodeType;
    }

    void NodeSearchPalette::SetAvailableNodeTypes(const std::vector<SearchableNodeInfo> &nodeTypes)
    {
        m_availableNodes = nodeTypes;
        if (m_isOpen)
        {
            UpdateSearchResults();
        }
    }

    void NodeSearchPalette::RecordNodeUsage(const std::string &typeId)
    {
        // Find and increment use count
        for (auto &node : m_availableNodes)
        {
            if (node.typeId == typeId)
            {
                node.useCount++;
                break;
            }
        }
    }

    void NodeSearchPalette::GetOpenPosition(float &outX, float &outY) const
    {
        outX = m_openPosX;
        outY = m_openPosY;
    }

    void NodeSearchPalette::UpdateSearchResults()
    {
        m_filteredNodes.clear();

        std::string query(m_searchBuffer);
        // Convert query to lowercase for case-insensitive search
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return std::tolower(c); });

        // If query is empty, show all nodes
        if (query.empty())
        {
            m_filteredNodes = m_availableNodes;
        }
        else
        {
            // Filter and score nodes
            for (auto node : m_availableNodes)
            {
                float score = CalculateFuzzyScore(query, node.displayName);
                if (score > 0.0f)
                {
                    node.matchScore = score;
                    m_filteredNodes.push_back(node);
                }
            }
        }

        // Sort results
        SortSearchResults();
    }

    float NodeSearchPalette::CalculateFuzzyScore(const std::string &query, const std::string &target)
    {
        if (query.empty())
            return 1.0f;
        if (target.empty())
            return 0.0f;

        // Convert target to lowercase
        std::string lowerTarget = target;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        // Check if target contains the query as a substring
        size_t pos = lowerTarget.find(query);
        if (pos != std::string::npos)
        {
            // Higher score for matches at the beginning
            float positionBonus = 1.0f - (static_cast<float>(pos) / static_cast<float>(lowerTarget.length()));
            // Higher score for exact matches (shorter targets)
            float lengthBonus = static_cast<float>(query.length()) / static_cast<float>(lowerTarget.length());
            return positionBonus * 0.5f + lengthBonus * 0.5f;
        }

        // Fuzzy matching: check if all query characters appear in order in target
        size_t queryIdx = 0;
        size_t targetIdx = 0;
        int matchedChars = 0;

        while (queryIdx < query.length() && targetIdx < lowerTarget.length())
        {
            if (query[queryIdx] == lowerTarget[targetIdx])
            {
                matchedChars++;
                queryIdx++;
            }
            targetIdx++;
        }

        if (queryIdx == query.length())
        {
            // All characters matched in order
            float completeness = static_cast<float>(matchedChars) / static_cast<float>(query.length());
            float compactness = static_cast<float>(matchedChars) / static_cast<float>(targetIdx);
            return completeness * 0.3f + compactness * 0.2f; // Lower score than substring match
        }

        return 0.0f; // No match
    }

    void NodeSearchPalette::SortSearchResults()
    {
        std::sort(m_filteredNodes.begin(),
            m_filteredNodes.end(),
            [](const SearchableNodeInfo &a, const SearchableNodeInfo &b) {
                // Sort by: 1. Use count (descending), 2. Match score (descending), 3. Display name (ascending)
                if (a.useCount != b.useCount)
                    return a.useCount > b.useCount;
                if (a.matchScore != b.matchScore)
                    return a.matchScore > b.matchScore;
                return a.displayName < b.displayName;
            });
    }
} // namespace VisionCraft::UI::Widgets
