#pragma once

#include "Layer.h"

namespace VisionCraft
{
    /**
     * @brief Layer that provides a property panel for editing node parameters.
     *
     * PropertyPanelLayer displays and allows editing of properties for the currently
     * selected node. It provides a user-friendly interface for modifying node parameters,
     * settings, and configuration options. The panel typically appears as a dockable
     * window within the main interface.
     */
    class PropertyPanelLayer : public Core::Layer
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
         * @param event The event to handle
         *
         * Processes events related to node selection changes and property updates.
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the property panel state.
         * @param deltaTime Time elapsed since the last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders the property panel interface.
         *
         * Displays the properties of the currently selected node in an editable
         * form, allowing users to modify node parameters and settings.
         */
        void OnRender() override;

    private:
        // TODO: Handle node selection through events or shared state
    };
} // namespace VisionCraft