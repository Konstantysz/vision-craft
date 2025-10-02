#pragma once

#include "Event.h"

namespace Core
{
    /**
     * @brief Base class for application layers.
     */
    class Layer
    {
    public:
        /**
         * @brief Called when an event occurs.
         * @param event Event to handle
         */
        virtual void OnEvent(Event &event)
        {
        }

        /**
         * @brief Called every frame to update the layer.
         * @param deltaTime Time elapsed since last update
         */
        virtual void OnUpdate(float deltaTime)
        {
        }

        /**
         * @brief Called every frame to render the layer.
         */
        virtual void OnRender()
        {
        }
    };
} // namespace Core
