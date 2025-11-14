#pragma once

#include "Application.h"
#include "Nodes/Core/NodeEditor.h"

// TODO: Move all member variable initializations from declarations to constructors across entire repository
// Example: Instead of `bool flag = true;` in class declaration, use `flag(true)` in constructor initializer list
// This improves code clarity and follows C++ best practices for initialization order

namespace VisionCraft::App
{
    /**
     * @brief Application with node editor and ImGui integration.
     * @note Sets up all required layers automatically.
     */
    class VisionCraftApplication : public Kappa::Application
    {
    public:
        /**
         * @brief Constructs the application.
         * @param specification Application configuration
         */
        VisionCraftApplication(
            const Kappa::ApplicationSpecification &specification = Kappa::ApplicationSpecification());

        /**
         * @brief Virtual destructor that ensures proper ImGui cleanup.
         */
        virtual ~VisionCraftApplication();

        /**
         * @brief Returns the node editor instance.
         * @return Node editor
         */
        [[nodiscard]] Nodes::NodeEditor &GetNodeEditor();

        /**
         * @brief Returns the node editor instance.
         * @return Node editor
         */
        [[nodiscard]] const Nodes::NodeEditor &GetNodeEditor() const;

    protected:
        /**
         * @brief Initializes ImGui frame.
         */
        void BeginFrame() override;

        /**
         * @brief Renders and presents ImGui.
         */
        void EndFrame() override;

    private:
        /**
         * @brief Initializes ImGui context and backends.
         */
        void InitializeImGui();

        /**
         * @brief Shuts down ImGui.
         */
        void ShutdownImGui();

        bool imguiInitialized = false; ///< Flag indicating if ImGui has been initialized
        Nodes::NodeEditor nodeEditor;  ///< Shared node editor instance accessed by all layers
    };
} // namespace VisionCraft::App
