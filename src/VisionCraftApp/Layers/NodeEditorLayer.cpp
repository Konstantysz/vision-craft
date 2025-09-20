#include "NodeEditorLayer.h"

#include <imgui.h>

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

        // TODO: Implement node rendering and interaction
        // For now, just show a placeholder
        ImGui::Text("Node Editor - TODO: Implement nodes");

        if (ImGui::Button("Add Node"))
        {
            // TODO: Add node creation logic
        }

        ImGui::End();
    }
} // namespace VisionCraft