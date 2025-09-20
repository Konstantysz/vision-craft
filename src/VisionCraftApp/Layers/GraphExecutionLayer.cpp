#include "GraphExecutionLayer.h"

#include <imgui.h>

namespace VisionCraft
{
    void GraphExecutionLayer::OnEvent(Core::Event &event)
    {
    }

    void GraphExecutionLayer::OnUpdate(float deltaTime)
    {
    }

    void GraphExecutionLayer::OnRender()
    {
        ImGui::Begin("Execution");

        if (ImGui::Button("Execute Graph"))
        {
            isExecuting = !isExecuting;
            // TODO: Implement graph execution
        }

        ImGui::SameLine();

        if (ImGui::Button("Show Results"))
        {
            showResultsWindow = !showResultsWindow;
        }

        if (isExecuting)
        {
            ImGui::Text("Executing...");
            // TODO: Show execution progress
        }

        ImGui::End();

        if (showResultsWindow)
        {
            ImGui::Begin("Results", &showResultsWindow);

            ImGui::Text("Execution Results");
            ImGui::Separator();

            // TODO: Show actual results/images
            ImGui::Text("No results to display");

            ImGui::End();
        }
    }
} // namespace VisionCraft