#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class CanvasLayer : public Core::Layer
    {
    public:
        CanvasLayer() = default;
        virtual ~CanvasLayer() = default;

        void OnEvent(Core::Event& event) override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

    private:
        float zoomLevel = 1.0f;
        float panX = 0.0f;
        float panY = 0.0f;
        float gridSize = 20.0f;
        bool showGrid = true;
    };
}