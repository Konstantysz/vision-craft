#pragma once

#include "Layer.h"

namespace VisionCraft
{
    /**
     * @brief Layer that provides the main docking space for the application.
     *
     * DockSpaceLayer creates a fullscreen ImGui dockspace that serves as the main
     * container for all other UI panels in the application. It provides the foundation
     * for the dockable interface where users can arrange panels like Canvas, Properties,
     * and other tools. The layer also includes a menu bar for application-wide actions.
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
         * @brief Handles events for the dockspace.
         * @param event The event to handle
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the dockspace state.
         * @param deltaTime Time elapsed since the last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders the dockspace and menu bar.
         *
         * Creates a fullscreen dockspace window that fills the entire viewport
         * and provides docking targets for other panels. Also renders the main
         * menu bar with application commands.
         */
        void OnRender() override;

    private:
        bool dockspaceOpen = true; ///< Flag indicating if the dockspace is open
    };
} // namespace VisionCraft