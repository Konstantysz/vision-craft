#include "CanvasLayer.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

namespace VisionCraft
{
    void CanvasLayer::OnEvent(Core::Event &event)
    {
    }

    void CanvasLayer::OnUpdate(float deltaTime)
    {
    }

    void CanvasLayer::OnRender()
    {
        ImGui::Begin("Canvas");

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

        ImGui::End();
    }
} // namespace VisionCraft