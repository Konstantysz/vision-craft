#pragma once

#include "Application.h"
#include "NodeEditor.h"

namespace VisionCraft
{
    /**
     * @brief VisionCraft-specific application implementation with ImGui integration.
     *
     * VisionCraftApplication extends the Core::Application class to provide a complete
     * application implementation for the VisionCraft node editor. It automatically sets up
     * the required layers (DockSpace, Canvas, NodeEditor, PropertyPanel, GraphExecution)
     * and handles ImGui initialization and rendering through the BeginFrame/EndFrame hooks.
     *
     * This class provides a ready-to-use application that implements the layered architecture
     * pattern with ImGui-based GUI rendering.
     */
    class VisionCraftApplication : public Core::Application
    {
    public:
        /**
         * @brief Constructs a VisionCraftApplication with the given specification.
         * @param specification Configuration options for the application
         *
         * The constructor automatically sets up all required layers for the node editor
         * interface in the correct order.
         */
        VisionCraftApplication(const Core::ApplicationSpecification &specification = Core::ApplicationSpecification());

        /**
         * @brief Virtual destructor that ensures proper ImGui cleanup.
         */
        virtual ~VisionCraftApplication();

        /**
         * @brief Gets the shared node editor instance.
         * @return Reference to the node editor
         */
        [[nodiscard]] Engine::NodeEditor &GetNodeEditor();

        /**
         * @brief Gets the shared node editor instance (const version).
         * @return Const reference to the node editor
         */
        [[nodiscard]] const Engine::NodeEditor &GetNodeEditor() const;

    protected:
        /**
         * @brief Called at the beginning of each frame to initialize ImGui.
         *
         * This method handles ImGui context creation and frame setup. ImGui is
         * initialized lazily on the first frame to ensure proper OpenGL context.
         */
        void BeginFrame() override;

        /**
         * @brief Called at the end of each frame to render and present ImGui.
         *
         * This method handles ImGui rendering, draw data submission, and
         * multi-viewport support if enabled.
         */
        void EndFrame() override;

    private:
        /**
         * @brief Initializes the ImGui context and backends.
         *
         * Sets up ImGui with OpenGL3 and GLFW backends, enables docking and viewports,
         * and configures the dark theme.
         */
        void InitializeImGui();

        /**
         * @brief Shuts down ImGui and cleans up resources.
         *
         * Properly destroys ImGui backends and context. Note: there may be
         * harmless GLFW errors during shutdown due to order dependencies.
         */
        void ShutdownImGui();

        bool imguiInitialized = false; ///< Flag indicating if ImGui has been initialized
        Engine::NodeEditor nodeEditor; ///< Shared node editor instance accessed by all layers
    };
} // namespace VisionCraft