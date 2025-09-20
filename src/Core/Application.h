#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Layer.h"
#include "Window.h"

namespace Core
{
    /**
     * @brief Configuration structure for creating an application.
     *
     * This structure contains all the necessary configuration options for initializing
     * an application, including the application name and window specifications.
     */
    struct ApplicationSpecification
    {
        std::string name = "Application";        ///< Name of the application
        WindowSpecification windowSpecification; ///< Window configuration options
    };

    /**
     * @brief Abstract base class for all applications.
     *
     * Application serves as the foundation for creating GUI applications with layer-based architecture.
     * It manages the main application loop, window lifecycle, and layer stack. Derived classes should
     * override the virtual BeginFrame() and EndFrame() methods to implement custom frame processing
     * (e.g., ImGui initialization and rendering).
     */
    class Application
    {
    public:
        /**
         * @brief Constructs an application with the given specification.
         * @param specification Configuration options for the application
         */
        Application(const ApplicationSpecification &specification = ApplicationSpecification());

        /**
         * @brief Virtual destructor for proper cleanup of derived classes.
         */
        virtual ~Application();

        /**
         * @brief Starts the main application loop.
         *
         * This method runs the main loop that handles events, updates layers,
         * and renders frames until the application is stopped.
         */
        void Run();

        /**
         * @brief Stops the application loop.
         *
         * This method sets the running flag to false, causing the main loop to exit.
         */
        void Stop();

    protected:
        /**
         * @brief Adds a layer to the application's layer stack.
         * @tparam TLayer Type of layer to add (must derive from Layer)
         *
         * The layer will be default-constructed and added to the end of the layer stack.
         * Layers are processed in the order they are added.
         */
        template<typename TLayer>
            requires std::is_base_of_v<Layer, TLayer>
        void PushLayer()
        {
            static_assert(std::is_default_constructible_v<TLayer>, "Layer must be default constructible");
            layerStack.push_back(std::make_unique<TLayer>());
        }

        /**
         * @brief Gets the current framebuffer size.
         * @return 2D vector containing width and height of the framebuffer
         */
        glm::vec2 GetFramebufferSize() const;

        /**
         * @brief Called at the beginning of each frame.
         *
         * Override this method in derived classes to implement custom frame initialization
         * logic (e.g., ImGui frame setup).
         */
        virtual void BeginFrame()
        {
        }

        /**
         * @brief Called at the end of each frame.
         *
         * Override this method in derived classes to implement custom frame finalization
         * logic (e.g., ImGui rendering and presentation).
         */
        virtual void EndFrame()
        {
        }

        /**
         * @brief Gets the singleton instance of the application.
         * @return Reference to the current application instance
         */
        static Application &Get();

        /**
         * @brief Gets the current time in seconds.
         * @return Current time as a float
         */
        static float GetTime();

    private:
        ApplicationSpecification specification; ///< Application configuration
        std::vector<std::unique_ptr<Layer>> layerStack; ///< Stack of application layers
        std::shared_ptr<Window> window; ///< Main application window
        bool isRunning = false; ///< Flag indicating if the application is running
    };
} // namespace Core