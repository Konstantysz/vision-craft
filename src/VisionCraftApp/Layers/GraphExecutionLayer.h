#pragma once

#include "Layer.h"
#include "Node.h"
#include "NodeEditor.h"

#include <vector>

namespace VisionCraft
{
    /**
     * @brief Layer that handles execution of the node graph and displays results.
     *
     * GraphExecutionLayer manages the execution of the computer vision pipeline
     * defined by the node graph. It handles running the processing chain, managing
     * execution state, and displaying results in a dedicated results window. This
     * layer provides the runtime functionality for the node editor.
     */
    class GraphExecutionLayer : public Core::Layer
    {
    public:
        /**
         * @brief Constructor that subscribes to graph execution events.
         */
        GraphExecutionLayer();

        /**
         * @brief Virtual destructor.
         */
        virtual ~GraphExecutionLayer() = default;

        /**
         * @brief Handles graph execution events.
         * @param event The event to handle
         *
         * Processes events related to graph execution triggers and result display.
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the graph execution state.
         * @param deltaTime Time elapsed since the last update
         *
         * Monitors execution progress and handles asynchronous processing updates.
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders the graph execution interface and results.
         *
         * Displays execution controls, progress indicators, and results window
         * showing the output of the computer vision processing pipeline.
         */
        void OnRender() override;

    private:
        /**
         * @brief Executes the node graph.
         *
         * This method is called when a GraphExecuteEvent is published.
         * It delegates to NodeEditor::Execute() which handles the execution logic.
         */
        void ExecuteGraph();

        bool isExecuting = false;       ///< Whether the graph is currently executing
        bool showResultsWindow = false; ///< Whether to display the results window
    };
} // namespace VisionCraft