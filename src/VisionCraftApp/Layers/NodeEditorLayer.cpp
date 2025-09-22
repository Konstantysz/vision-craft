#include "NodeEditorLayer.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/LoadImageNode.h"
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
            auto testNode = std::make_unique<Engine::LoadImageNode>(nextNodeId++, "Test Image");
            Engine::NodeId nodeId = testNode->GetId();
            nodeEditor.AddNode(std::move(testNode));
            nodePositions[nodeId] = { 100.0f, 100.0f };
        }

        RenderNodes();

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

        ImU32 nodeColor = IM_COL32(60, 60, 60, 255);
        ImU32 borderColor = IM_COL32(100, 100, 100, 255);
        ImU32 titleColor = IM_COL32(80, 80, 120, 255);

        drawList->AddRectFilled(
            worldPos, ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y), nodeColor, 8.0f * zoomLevel);

        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y),
            borderColor,
            8.0f * zoomLevel,
            0,
            2.0f * zoomLevel);

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
} // namespace VisionCraft