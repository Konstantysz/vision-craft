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

            // Thread-safe progress reading - atomics are lock-free
            int current = currentNode.load(std::memory_order_relaxed);
            int total = totalNodes.load(std::memory_order_relaxed);
            std::string name;
            {
                std::lock_guard<std::mutex> lock(nameMutex);
                name = currentNodeName;
            }

            if (total > 0)
            {
                float progress = static_cast<float>(current) / static_cast<float>(total);
                char overlay[64];
                snprintf(overlay, sizeof(overlay), "%d/%d: %s", current, total, name.c_str());
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
        currentNode.store(0, std::memory_order_relaxed);
        totalNodes.store(0, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(nameMutex);
            currentNodeName = "Initializing...";
        }

        executionFuture = nodeEditor.ExecuteAsync([this](int current, int total, const std::string &name) {
            // Use atomics for numeric values to avoid mutex overhead
            currentNode.store(current, std::memory_order_relaxed);
            totalNodes.store(total, std::memory_order_relaxed);
            // Only lock for string update
            {
                std::lock_guard<std::mutex> lock(nameMutex);
                currentNodeName = name;
            }
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
