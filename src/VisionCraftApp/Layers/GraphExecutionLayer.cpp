#include "GraphExecutionLayer.h"

#include <imgui.h>

#include "Application.h"
#include "Events/GraphExecuteEvent.h"
#include "Logger.h"
#include "VisionCraftApplication.h"

namespace VisionCraft
{
    GraphExecutionLayer::GraphExecutionLayer()
    {
        Core::Application::Get().GetEventBus().Subscribe<GraphExecuteEvent>(
            [this]([[maybe_unused]] const GraphExecuteEvent &event) { ExecuteGraph(); });
    }

    void GraphExecutionLayer::OnEvent([[maybe_unused]] Core::Event &event)
    {
    }

    void GraphExecutionLayer::OnUpdate([[maybe_unused]] float deltaTime)
    {
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

    void GraphExecutionLayer::ExecuteGraph()
    {
        LOG_INFO("Graph execution triggered via EventBus!");
        isExecuting = true;
        showResultsWindow = true;

        auto &app = static_cast<VisionCraftApplication &>(Core::Application::Get());
        auto &nodeEditor = app.GetNodeEditor();

        // Execute the graph (backend handles all the complexity)
        const bool success = nodeEditor.Execute();

        isExecuting = false;

        if (!success)
        {
            LOG_ERROR("Graph execution failed");
        }
    }
} // namespace VisionCraft