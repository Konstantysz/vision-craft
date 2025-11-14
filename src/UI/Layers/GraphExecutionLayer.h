#pragma once

#include "Layer.h"
#include "Nodes/Core/Node.h"
#include "Nodes/Core/NodeEditor.h"

#include <vector>

namespace VisionCraft::UI::Layers
{
    /**
     * @brief Layer for graph execution and results display.
     */
    class GraphExecutionLayer : public Kappa::Layer
    {
    public:
        /**
         * @brief Constructor that subscribes to graph execution events.
         * @param nodeEditor Reference to the shared node editor instance
         */
        explicit GraphExecutionLayer(Nodes::NodeEditor &nodeEditor);

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

        Nodes::NodeEditor &nodeEditor;  ///< Reference to the shared node editor instance
        bool isExecuting = false;       ///< Whether the graph is currently executing
        bool showResultsWindow = false; ///< Whether to display the results window
    };
} // namespace VisionCraft::UI::Layers
