#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class ImGuiEndLayer : public Core::Layer
    {
    public:
        ImGuiEndLayer() = default;
        virtual ~ImGuiEndLayer() = default;

        void OnEvent(Core::Event& event) override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
    };
}