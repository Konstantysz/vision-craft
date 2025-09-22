#include "NodeEditorLayer.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/ThresholdNode.h"

namespace VisionCraft
{
    void NodeEditorLayer::OnEvent(Core::Event &event)
    {
    }

    void NodeEditorLayer::OnUpdate(float deltaTime)
    {
    }

    void NodeEditorLayer::OnRender()
    {
        ImGui::Begin("Node Editor");

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();

        currentCanvasPos = canvas_pos;

        if (canvas_size.x < 50.0f)
        {
            canvas_size.x = 50.0f;
        }

        if (canvas_size.y < 50.0f)
        {
            canvas_size.y = 50.0f;
        }

        if (showGrid)
        {
            ImU32 grid_color = IM_COL32(200, 200, 200, 40);
            float grid_step = gridSize * zoomLevel;

            for (float x = fmodf(panX, grid_step); x < canvas_size.x; x += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y),
                    ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
                    grid_color);
            }

            for (float y = fmodf(panY, grid_step); y < canvas_size.y; y += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y),
                    ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
                    grid_color);
            }
        }

        ImGuiIO &io = ImGui::GetIO();
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            panX += io.MouseDelta.x;
            panY += io.MouseDelta.y;
        }

        if (ImGui::IsWindowHovered())
        {
            if (io.MouseWheel != 0.0f)
            {
                zoomLevel += io.MouseWheel * 0.1f;
                zoomLevel = std::clamp(zoomLevel, 0.1f, 5.0f);
            }
        }

        if (nodeEditor.GetNodeIds().empty())
        {
            auto starterNode = std::make_unique<Engine::ImageInputNode>(nextNodeId++);
            Engine::NodeId nodeId = starterNode->GetId();
            nodeEditor.AddNode(std::move(starterNode));
            nodePositions[nodeId] = { 100.0f, 100.0f };
        }

        // Handle mouse interactions for node selection and dragging
        HandleMouseInteractions();

        RenderNodes();

        // Render context menu
        RenderContextMenu();

        ImGui::End();
    }

    void NodeEditorLayer::RenderNodes()
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        for (Engine::NodeId nodeId : nodeEditor.GetNodeIds())
        {
            Engine::Node *node = nodeEditor.GetNode(nodeId);
            if (node && nodePositions.find(nodeId) != nodePositions.end())
            {
                RenderNode(node, nodePositions[nodeId]);
            }
        }
    }

    void NodeEditorLayer::RenderNode(Engine::Node *node, const NodePosition &nodePos)
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        ImVec2 worldPos =
            ImVec2(canvasPos.x + (nodePos.x + panX) * zoomLevel, canvasPos.y + (nodePos.y + panY) * zoomLevel);

        ImVec2 nodeSize = ImVec2(150.0f * zoomLevel, 80.0f * zoomLevel);

        // Check if this node is selected
        bool isSelected = (node->GetId() == selectedNodeId);

        ImU32 nodeColor = IM_COL32(60, 60, 60, 255);
        ImU32 borderColor = isSelected ? IM_COL32(255, 165, 0, 255) : IM_COL32(100, 100, 100, 255); // Orange if selected
        ImU32 titleColor = IM_COL32(80, 80, 120, 255);

        drawList->AddRectFilled(
            worldPos, ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y), nodeColor, 8.0f * zoomLevel);

        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y),
            borderColor,
            8.0f * zoomLevel,
            0,
            isSelected ? 3.0f * zoomLevel : 2.0f * zoomLevel); // Thicker border if selected

        float titleHeight = 25.0f * zoomLevel;
        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + titleHeight),
            titleColor,
            8.0f * zoomLevel,
            ImDrawFlags_RoundCornersTop);

        if (zoomLevel > 0.5f)
        {
            ImVec2 textPos = ImVec2(worldPos.x + 8.0f * zoomLevel, worldPos.y + 4.0f * zoomLevel);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), node->GetName().c_str());
        }
    }

    bool NodeEditorLayer::IsMouseOverNode(const ImVec2& mousePos, const NodePosition& nodePos, const ImVec2& nodeSize) const
    {
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        // Convert node position to screen coordinates
        ImVec2 worldPos = ImVec2(
            canvasPos.x + (nodePos.x + panX) * zoomLevel,
            canvasPos.y + (nodePos.y + panY) * zoomLevel
        );

        // Check if mouse is within node bounds
        return mousePos.x >= worldPos.x && mousePos.x <= worldPos.x + nodeSize.x &&
               mousePos.y >= worldPos.y && mousePos.y <= worldPos.y + nodeSize.y;
    }

    void NodeEditorLayer::HandleMouseInteractions()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        // Only handle interactions when window is hovered and not panning
        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
            return;

        ImVec2 nodeSize = ImVec2(150.0f * zoomLevel, 80.0f * zoomLevel);

        // Handle right-click for context menu
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            Engine::NodeId clickedNodeId = -1;

            // Check if right-clicked on a node
            auto nodeIds = nodeEditor.GetNodeIds();
            for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
            {
                Engine::NodeId nodeId = *it;
                if (nodePositions.find(nodeId) != nodePositions.end())
                {
                    if (IsMouseOverNode(mousePos, nodePositions[nodeId], nodeSize))
                    {
                        clickedNodeId = nodeId;
                        break;
                    }
                }
            }

            if (clickedNodeId == -1)
            {
                // Right-clicked on empty space, show context menu
                showContextMenu = true;
                contextMenuPos = mousePos;
                ImGui::OpenPopup("NodeContextMenu");
            }
        }

        // Handle mouse click for selection
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            Engine::NodeId clickedNodeId = -1;

            // Check all nodes to see which one was clicked (iterate in reverse for top-to-bottom)
            auto nodeIds = nodeEditor.GetNodeIds();
            for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
            {
                Engine::NodeId nodeId = *it;
                if (nodePositions.find(nodeId) != nodePositions.end())
                {
                    if (IsMouseOverNode(mousePos, nodePositions[nodeId], nodeSize))
                    {
                        clickedNodeId = nodeId;
                        break; // Take the first (topmost) node hit
                    }
                }
            }

            if (clickedNodeId != -1)
            {
                // Select the clicked node and start dragging
                selectedNodeId = clickedNodeId;
                isDragging = true;

                // Calculate drag offset (mouse position relative to node position)
                const NodePosition& nodePos = nodePositions[clickedNodeId];
                ImVec2 worldPos = ImVec2(
                    canvasPos.x + (nodePos.x + panX) * zoomLevel,
                    canvasPos.y + (nodePos.y + panY) * zoomLevel
                );
                dragOffset = ImVec2(mousePos.x - worldPos.x, mousePos.y - worldPos.y);
            }
            else
            {
                // Clicked on empty space, deselect
                selectedNodeId = -1;
                isDragging = false;
            }
        }

        // Handle dragging
        if (isDragging && selectedNodeId != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            // Update node position based on mouse position minus offset
            ImVec2 newWorldPos = ImVec2(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);

            // Convert back to local node coordinates
            nodePositions[selectedNodeId].x = (newWorldPos.x - canvasPos.x) / zoomLevel - panX;
            nodePositions[selectedNodeId].y = (newWorldPos.y - canvasPos.y) / zoomLevel - panY;
        }

        // Stop dragging when mouse is released
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            isDragging = false;
        }
    }

    void NodeEditorLayer::RenderContextMenu()
    {
        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            ImGui::Text("Add Node");
            ImGui::Separator();

            if (ImGui::MenuItem("Image Input"))
            {
                CreateNodeAtPosition("ImageInput", contextMenuPos);
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Image Output"))
            {
                CreateNodeAtPosition("ImageOutput", contextMenuPos);
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Grayscale"))
            {
                CreateNodeAtPosition("Grayscale", contextMenuPos);
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Canny Edge Detection"))
            {
                CreateNodeAtPosition("CannyEdge", contextMenuPos);
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Threshold"))
            {
                CreateNodeAtPosition("Threshold", contextMenuPos);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void NodeEditorLayer::CreateNodeAtPosition(const std::string& nodeType, const ImVec2& position)
    {
        // Use stored canvas position for consistent coordinate calculation
        ImVec2 canvasPos = currentCanvasPos;

        // Convert screen position to world coordinates
        float worldX = (position.x - canvasPos.x) / zoomLevel - panX;
        float worldY = (position.y - canvasPos.y) / zoomLevel - panY;

        // Center the node at the click position (subtract half node size)
        worldX -= 75.0f; // Half of node width (150 / 2)
        worldY -= 40.0f; // Half of node height (80 / 2)

        Engine::NodeId nodeId = nextNodeId++;
        std::unique_ptr<Engine::Node> newNode;

        if (nodeType == "ImageInput")
        {
            newNode = std::make_unique<Engine::ImageInputNode>(nodeId, "Image Input");
        }
        else if (nodeType == "ImageOutput")
        {
            newNode = std::make_unique<Engine::ImageOutputNode>(nodeId, "Image Output");
        }
        else if (nodeType == "Grayscale")
        {
            newNode = std::make_unique<Engine::GrayscaleNode>(nodeId, "Grayscale");
        }
        else if (nodeType == "CannyEdge")
        {
            newNode = std::make_unique<Engine::CannyEdgeNode>(nodeId, "Canny Edge");
        }
        else if (nodeType == "Threshold")
        {
            newNode = std::make_unique<Engine::ThresholdNode>(nodeId, "Threshold");
        }

        if (newNode)
        {
            nodeEditor.AddNode(std::move(newNode));
            nodePositions[nodeId] = { worldX, worldY };

            // Select the newly created node
            selectedNodeId = nodeId;
        }
    }
} // namespace VisionCraft