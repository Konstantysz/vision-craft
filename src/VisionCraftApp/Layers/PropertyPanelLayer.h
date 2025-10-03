#pragma once

#include "Layer.h"

namespace VisionCraft
{
    /**
     * @brief Layer for editing node properties.
     */
    class PropertyPanelLayer : public Kappa::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        PropertyPanelLayer() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~PropertyPanelLayer() = default;

        /**
         * @brief Handles property panel events.
         * @param event Event to handle
         */
        void OnEvent(Kappa::Event &event) override;

        /**
         * @brief Updates property panel state.
         * @param deltaTime Time since last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders property panel.
         */
        void OnRender() override;

    private:
        // TODO: Handle node selection through events or shared state
    };
} // namespace VisionCraft