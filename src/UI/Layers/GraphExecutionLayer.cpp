#include "UI/Layers/GraphExecutionLayer.h"

#include <imgui.h>

#include "UI/Events/GraphExecuteEvent.h"
#include "Application.h"
#include "Logger.h"

namespace VisionCraft::UI::Layers
{
    GraphExecutionLayer::GraphExecutionLayer(Nodes::NodeEditor &nodeEditor) : nodeEditor(nodeEditor)
    {
        Kappa::Application::Get().GetEventBus().Subscribe<Events::GraphExecuteEvent>(
            [this]([[maybe_unused]] const Events::GraphExecuteEvent &event) { ExecuteGraph(); });
    }

    void GraphExecutionLayer::OnEvent([[maybe_unused]] Kappa::Event &event)
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
            Kappa::Application::Get().GetEventBus().Publish(Events::GraphExecuteEvent{});
        }

        ImGui::SameLine();

        if (ImGui::Button("Show Results"))
        {
            showResultsWindow = !showResultsWindow;
        }

        if (isExecuting)
        {
            // TODO: Show execution progress
            ImGui::Text("Executing...");
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

        const bool success = nodeEditor.Execute();

        isExecuting = false;

        if (!success)
        {
            LOG_ERROR("Graph execution failed");
        }
    }
} // namespace VisionCraft::UI::Layers
