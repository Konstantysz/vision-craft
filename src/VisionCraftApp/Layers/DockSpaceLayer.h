#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class DockSpaceLayer : public Core::Layer
    {
    public:
        DockSpaceLayer() = default;

        virtual ~DockSpaceLayer() = default;

        void OnEvent(Core::Event &event) override;

        void OnUpdate(float deltaTime) override;

        void OnRender() override;

    private:
        bool dockspaceOpen = true;
    };
} // namespace VisionCraft