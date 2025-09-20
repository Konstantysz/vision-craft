#pragma once

#include "Layer.h"

namespace VisionCraft
{
    class PropertyPanelLayer : public Core::Layer
    {
    public:
        PropertyPanelLayer() = default;

        virtual ~PropertyPanelLayer() = default;

        void OnEvent(Core::Event &event) override;

        void OnUpdate(float deltaTime) override;

        void OnRender() override;

    private:
        // TODO: Handle node selection through events or shared state
    };
} // namespace VisionCraft