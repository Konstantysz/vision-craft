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
        if (isExecuting && executionFuture.valid())
        {
            // Check if execution is finished (timeout of 0 means just check status)
            if (executionFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                bool success = executionFuture.get();
                isExecuting = false;

                if (success)
                {
                    LOG_INFO("Async graph execution completed successfully");
                }
                else
                {
                    LOG_ERROR("Async graph execution failed or was cancelled");
                }
            }
        }
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
            ImGui::Separator();
            ImGui::Text("Executing Graph...");

            // Thread-safe progress reading
            int current, total;
            std::string name;
            {
                std::lock_guard<std::mutex> lock(progressMutex);
                current = currentNode;
                total = totalNodes;
                name = currentNodeName;
            }

            if (total > 0)
            {
                float progress = static_cast<float>(current) / static_cast<float>(total);
                char overlay[64];
                sprintf_s(overlay, "%d/%d: %s", current, total, name.c_str());
                ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), overlay);
            }
            else
            {
                ImGui::ProgressBar(0.0f, ImVec2(0.0f, 0.0f), "Preparing...");
            }

            if (ImGui::Button("Cancel Execution"))
            {
                CancelExecution();
            }
        }
        else
        {
            // Only show status if not executing
            ImGui::Text("Ready");
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

        if (isExecuting)
        {
            LOG_WARN("Graph is already executing");
            return;
        }

        isExecuting = true;
        showResultsWindow = true;
        currentNode = 0;
        totalNodes = 0;
        currentNodeName = "Initializing...";

        executionFuture = nodeEditor.ExecuteAsync([this](int current, int total, const std::string &name) {
            std::lock_guard<std::mutex> lock(progressMutex);
            currentNode = current;
            totalNodes = total;
            currentNodeName = name;
        });
    }

    void GraphExecutionLayer::CancelExecution()
    {
        if (isExecuting)
        {
            LOG_INFO("Cancelling graph execution...");
            nodeEditor.CancelExecution();
        }
    }
} // namespace VisionCraft::UI::Layers
