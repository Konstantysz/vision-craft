#include "PropertyPanelLayer.h"

#include <imgui.h>

namespace VisionCraft
{
    void PropertyPanelLayer::OnEvent(Kappa::Event &event)
    {
    }

    void PropertyPanelLayer::OnUpdate(float deltaTime)
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
} // namespace VisionCraft