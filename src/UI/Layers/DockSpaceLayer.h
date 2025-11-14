#pragma once

#include "Editor/Persistence/RecentFilesManager.h"
#include "Layer.h"

namespace VisionCraft::UI::Layers
{
    /**
     * @brief Layer providing fullscreen dockspace and menu bar.
     */
    class DockSpaceLayer : public Kappa::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        DockSpaceLayer();

        /**
         * @brief Virtual destructor.
         */
        virtual ~DockSpaceLayer() = default;

        /**
         * @brief Handles dockspace events.
         * @param event Event to handle
         */
        void OnEvent(Kappa::Event &event) override;

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
        void RenderFileMenu();
        void RenderRecentFilesMenu();

        bool dockspaceOpen = true; ///< Flag indicating if the dockspace is open
        bool isFirstFrame = true;  ///< Flag to detect first frame for default layout setup
        Editor::Persistence::RecentFilesManager recentFilesManager; ///< Manager for recent files
    };
} // namespace VisionCraft::UI::Layers
