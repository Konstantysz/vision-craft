#pragma once

#include "Layer.h"

namespace VisionCraft
{
    /**
     * @brief Layer providing fullscreen dockspace and menu bar.
     */
    class DockSpaceLayer : public Core::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        DockSpaceLayer() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~DockSpaceLayer() = default;

        /**
         * @brief Handles dockspace events.
         * @param event Event to handle
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates dockspace state.
         * @param deltaTime Time since last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders dockspace and menu bar.
         */
        void OnRender() override;

    private:
        bool dockspaceOpen = true; ///< Flag indicating if the dockspace is open
    };
} // namespace VisionCraft