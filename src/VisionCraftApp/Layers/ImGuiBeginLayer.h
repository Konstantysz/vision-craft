#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class ImGuiBeginLayer : public Core::Layer
    {
    public:
        ImGuiBeginLayer();
        virtual ~ImGuiBeginLayer();

        void OnEvent(Core::Event& event) override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

    private:
        void InitializeImGui();

        bool blockEvents = true;
        bool initialized = false;
    };
}