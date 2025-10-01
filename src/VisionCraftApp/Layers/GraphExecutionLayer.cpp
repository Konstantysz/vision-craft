#include "GraphExecutionLayer.h"

#include <imgui.h>

#include "Application.h"
#include "Events/GraphExecuteEvent.h"
#include "Logger.h"

namespace VisionCraft
{
    GraphExecutionLayer::GraphExecutionLayer()
    {
        Core::Application::Get().GetEventBus().Subscribe<GraphExecuteEvent>(
            [this](const GraphExecuteEvent &event) { ExecuteGraph(); });
    }

    void GraphExecutionLayer::OnEvent(Core::Event &event)
    {
    }

    void GraphExecutionLayer::OnUpdate(float deltaTime)
    {
    }

    void GraphExecutionLayer::ExecuteGraph()
    {
        LOG_INFO("Graph execution triggered via EventBus!");
        isExecuting = true;
        showResultsWindow = true;
        // TODO: Implement actual graph execution logic
    }

    void GraphExecutionLayer::OnRender()
    {
        ImGui::Begin("Execution");

        if (ImGui::Button("Execute Graph"))
        {
            Core::Application::Get().GetEventBus().Publish(GraphExecuteEvent{});
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