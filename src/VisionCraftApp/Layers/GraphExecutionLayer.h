#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class GraphExecutionLayer : public Core::Layer
    {
    public:
        GraphExecutionLayer() = default;

        virtual ~GraphExecutionLayer() = default;

        void OnEvent(Core::Event &event) override;

        void OnUpdate(float deltaTime) override;

        void OnRender() override;

    private:
        bool isExecuting = false;
        bool showResultsWindow = false;
    };
} // namespace VisionCraft