#include "UI/Layers/PropertyPanelLayer.h"

#include <imgui.h>

namespace VisionCraft::UI::Layers
{
    void PropertyPanelLayer::OnEvent([[maybe_unused]] Kappa::Event &event)
    {
    }

    void PropertyPanelLayer::OnUpdate([[maybe_unused]] float deltaTime)
    {
    }

    void PropertyPanelLayer::OnRender()
    {
        ImGui::Begin("Properties");

        // TODO: Show properties of selected node
        ImGui::Text("Properties Panel");
        ImGui::Separator();
        ImGui::Text("No node selected");

        ImGui::End();
    }
} // namespace VisionCraft::UI::Layers
