#pragma once

#include "Event.h"

namespace Core
{
    /**
     * @brief Abstract base class for application layers.
     *
     * Layer represents a single layer in the application's layer stack. Each layer
     * can handle events, update its state, and render content. Layers are processed
     * in the order they are added to the application, allowing for composable and
     * modular application architecture.
     *
     * Derived classes should override the virtual methods to implement specific
     * functionality for event handling, updates, and rendering.
     */
    class Layer
    {
    public:
        /**
         * @brief Virtual destructor for proper cleanup of derived classes.
         */
        virtual ~Layer() = default;

        /**
         * @brief Called when an event occurs.
         * @param event The event to handle
         *
         * Override this method to handle input events, window events, or other
         * application events. The default implementation does nothing.
         */
        virtual void OnEvent(Event &event)
        {
        }

        /**
         * @brief Called every frame to update the layer's state.
         * @param deltaTime Time elapsed since the last update in seconds
         *
         * Override this method to implement per-frame logic such as animations,
         * physics updates, or state changes. The default implementation does nothing.
         */
        virtual void OnUpdate(float deltaTime)
        {
        }

        /**
         * @brief Called every frame to render the layer's content.
         *
         * Override this method to implement rendering logic such as drawing UI elements,
         * 3D scenes, or other visual content. The default implementation does nothing.
         */
        virtual void OnRender()
        {
        }
    };
} // namespace Core
