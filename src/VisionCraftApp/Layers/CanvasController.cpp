#include "CanvasController.h"

#include <algorithm>
#include <cmath>

namespace VisionCraft
{
    CanvasController::CanvasController()
    {
    }

    void CanvasController::HandleInput(Kappa::Event &event, const ImVec2 &mousePos, bool isWindowHovered)
    {
    }

    void CanvasController::HandleImGuiInput(const ImGuiIO &io, bool isWindowHovered)
    {
        if (!isWindowHovered)
            return;

        if (io.MouseWheel != 0.0f)
        {
            ApplyZoomDelta(io.MouseWheel * Constants::Zoom::kStep);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            ApplyPanDelta(io.MouseDelta);
        }
    }

    void CanvasController::BeginCanvas(ImDrawList *drawList, const ImVec2 &canvasPos, const ImVec2 &canvasSize)
    {
        currentDrawList = drawList;
        currentCanvasPos = canvasPos;
        currentCanvasSize = EnsureMinimumCanvasSize(canvasSize);

        if (showGrid)
        {
            RenderGrid();
        }
    }

    void CanvasController::EndCanvas()
    {
        currentDrawList = nullptr;
    }

    ImVec2 CanvasController::ScreenToWorld(const ImVec2 &screenPos) const
    {
        return ImVec2((screenPos.x - currentCanvasPos.x - panX) / zoomLevel,
            (screenPos.y - currentCanvasPos.y - panY) / zoomLevel);
    }

    ImVec2 CanvasController::WorldToScreen(const ImVec2 &worldPos) const
    {
        return ImVec2(
            currentCanvasPos.x + (worldPos.x * zoomLevel + panX), currentCanvasPos.y + (worldPos.y * zoomLevel + panY));
    }

    void CanvasController::SetZoomLevel(float newZoom)
    {
        zoomLevel = ClampZoom(newZoom);
    }

    void CanvasController::ApplyZoomDelta(float delta)
    {
        SetZoomLevel(zoomLevel + delta);
    }

    void CanvasController::ZoomToFit(const ImVec2 &contentMin, const ImVec2 &contentMax, float padding)
    {
        if (currentCanvasSize.x <= 0 || currentCanvasSize.y <= 0)
            return;

        const auto contentSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);
        const auto availableSize = ImVec2(currentCanvasSize.x - padding * 2, currentCanvasSize.y - padding * 2);

        if (contentSize.x <= 0 || contentSize.y <= 0)
            return;

        const auto zoomX = availableSize.x / contentSize.x;
        const auto zoomY = availableSize.y / contentSize.y;
        const auto targetZoom = std::min(zoomX, zoomY);

        SetZoomLevel(targetZoom);

        const auto contentCenter = ImVec2((contentMin.x + contentMax.x) * 0.5f, (contentMin.y + contentMax.y) * 0.5f);
        CenterOn(contentCenter);
    }

    void CanvasController::SetPanOffset(const ImVec2 &offset)
    {
        panX = offset.x;
        panY = offset.y;
    }

    void CanvasController::ApplyPanDelta(const ImVec2 &delta)
    {
        panX += delta.x;
        panY += delta.y;
    }

    void CanvasController::CenterOn(const ImVec2 &worldPos)
    {
        const auto screenCenter = ImVec2(currentCanvasSize.x * 0.5f, currentCanvasSize.y * 0.5f);
        const auto worldScreenPos = ImVec2(worldPos.x * zoomLevel, worldPos.y * zoomLevel);
        SetPanOffset(ImVec2(screenCenter.x - worldScreenPos.x, screenCenter.y - worldScreenPos.y));
    }

    void CanvasController::ResetView()
    {
        zoomLevel = Constants::Zoom::kDefault;
        panX = 0.0f;
        panY = 0.0f;
    }

    bool CanvasController::IsRectangleVisible(const ImVec2 &worldMin, const ImVec2 &worldMax) const
    {
        const auto screenMin = WorldToScreen(worldMin);
        const auto screenMax = WorldToScreen(worldMax);

        return !(screenMax.x < currentCanvasPos.x || screenMin.x > currentCanvasPos.x + currentCanvasSize.x
                 || screenMax.y < currentCanvasPos.y || screenMin.y > currentCanvasPos.y + currentCanvasSize.y);
    }

    void CanvasController::GetVisibleWorldBounds(ImVec2 &outMin, ImVec2 &outMax) const
    {
        outMin = ScreenToWorld(currentCanvasPos);
        outMax =
            ScreenToWorld(ImVec2(currentCanvasPos.x + currentCanvasSize.x, currentCanvasPos.y + currentCanvasSize.y));
    }

    void CanvasController::RenderGrid()
    {
        if (!currentDrawList)
            return;

        const auto gridStep = gridSize * zoomLevel;

        for (float x = fmodf(panX, gridStep); x < currentCanvasSize.x; x += gridStep)
        {
            currentDrawList->AddLine(ImVec2(currentCanvasPos.x + x, currentCanvasPos.y),
                ImVec2(currentCanvasPos.x + x, currentCanvasPos.y + currentCanvasSize.y),
                Constants::Colors::Grid::kLines);
        }

        for (float y = fmodf(panY, gridStep); y < currentCanvasSize.y; y += gridStep)
        {
            currentDrawList->AddLine(ImVec2(currentCanvasPos.x, currentCanvasPos.y + y),
                ImVec2(currentCanvasPos.x + currentCanvasSize.x, currentCanvasPos.y + y),
                Constants::Colors::Grid::kLines);
        }
    }

    float CanvasController::ClampZoom(float zoom) const
    {
        return std::clamp(zoom, Constants::Zoom::kMin, Constants::Zoom::kMax);
    }

    ImVec2 CanvasController::EnsureMinimumCanvasSize(const ImVec2 &size) const
    {
        return ImVec2(std::max(size.x, Constants::Canvas::kMinSize), std::max(size.y, Constants::Canvas::kMinSize));
    }

} // namespace VisionCraft