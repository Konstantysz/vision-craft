#pragma once

namespace VisionCraft::UI::Widgets
{
    /**
     * @brief Helper for setting up ImGui docking layouts.
     */
    class DockingLayoutHelper
    {
    public:
        /**
         * @brief Sets up default docking layout for VisionCraft.
         * @note This creates a default layout if no ImGui .ini file exists.
         * @note Should be called once after ImGui initialization
         */
        static void SetupDefaultLayout();
    };
} // namespace VisionCraft::UI::Widgets
