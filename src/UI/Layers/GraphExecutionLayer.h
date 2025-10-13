#pragma once

#include "Layer.h"
#include "Node.h"
#include "NodeEditor.h"

#include <vector>

namespace VisionCraft
{
    /**
     * @brief Layer for graph execution and results display.
     */
    class GraphExecutionLayer : public Kappa::Layer
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
         * @brief Handles execution events.
         * @param event Event to handle
         */
        void OnEvent(Kappa::Event &event) override;

        /**
         * @brief Updates execution state.
         * @param deltaTime Time since last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders execution interface and results.
         */
        void OnRender() override;

    private:
        /**
         * @brief Executes node graph.
         */
        void ExecuteGraph();

        bool isExecuting = false;       ///< Whether the graph is currently executing
        bool showResultsWindow = false; ///< Whether to display the results window
    };
} // namespace VisionCraft
