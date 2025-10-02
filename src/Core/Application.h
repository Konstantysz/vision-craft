#pragma once
#include <memory>
#include <string>
#include <vector>

#include "EventBus.h"
#include "Layer.h"
#include "Window.h"

namespace Core
{
    /**
     * @brief Configuration for creating an application.
     */
    struct ApplicationSpecification
    {
        std::string name = "Application";        ///< Name of the application
        WindowSpecification windowSpecification; ///< Window configuration options
    };

    /**
     * @brief Base class for applications with layer-based architecture.
     * @note Only ONE instance should exist at a time (Service Locator pattern).
     */
    class Application
    {
    public:
        /**
         * @brief Constructs an application.
         * @param specification Application configuration
         */
        explicit Application(const ApplicationSpecification &specification = ApplicationSpecification());

        /**
         * @brief Virtual destructor for proper cleanup of derived classes.
         */
        virtual ~Application();

        /**
         * @brief Starts the main application loop.
         */
        void Run();

        /**
         * @brief Stops the application loop.
         */
        void Stop();

    protected:
        /**
         * @brief Adds a layer to the layer stack.
         * @tparam TLayer Layer type (must derive from Layer)
         */
        template<typename TLayer>
            requires std::is_base_of_v<Layer, TLayer>
        void PushLayer()
        {
            static_assert(std::is_default_constructible_v<TLayer>, "Layer must be default constructible");
            layerStack.push_back(std::make_unique<TLayer>());
        }

        /**
         * @brief Returns the framebuffer size.
         * @return Framebuffer size
         */
        [[nodiscard]] glm::vec2 GetFramebufferSize() const;

        /**
         * @brief Called at the beginning of each frame.
         */
        virtual void BeginFrame()
        {
        }

        /**
         * @brief Called at the end of each frame.
         */
        virtual void EndFrame()
        {
        }

    public:
        /**
         * @brief Returns the application instance.
         * @return Application instance
         */
        [[nodiscard]] static Application &Get();

        /**
         * @brief Returns the current time in seconds.
         * @return Current time
         */
        [[nodiscard]] static float GetTime();

        /**
         * @brief Returns the event bus.
         * @return Event bus
         */
        [[nodiscard]] EventBus &GetEventBus();

    private:
        ApplicationSpecification specification;         ///< Application configuration
        std::vector<std::unique_ptr<Layer>> layerStack; ///< Stack of application layers
        std::shared_ptr<Window> window;                 ///< Main application window
        bool isRunning = false;                         ///< Flag indicating if the application is running
        EventBus eventBus;                              ///< Event bus for inter-layer communication
    };
} // namespace Core