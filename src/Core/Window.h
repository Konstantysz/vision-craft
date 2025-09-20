#pragma once
#include <string>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <glm/glm.hpp>

namespace Core
{
    /**
     * @brief Configuration structure for creating a window.
     *
     * This structure contains all the necessary configuration options for initializing
     * a window, including dimensions, title, and various rendering options.
     */
    struct WindowSpecification
    {
        std::string title;               ///< Window title displayed in the title bar
        unsigned int width = 1280;      ///< Window width in pixels
        unsigned int height = 720;      ///< Window height in pixels
        bool isResizable = true;         ///< Whether the window can be resized by the user
        bool vSync = true;               ///< Whether vertical sync is enabled
    };

    /**
     * @brief Cross-platform window wrapper using GLFW.
     *
     * Window provides a high-level interface for creating and managing application windows.
     * It handles GLFW window creation, OpenGL context setup, and basic window operations.
     * The window automatically initializes OpenGL using GLAD when created.
     */
    class Window
    {
    public:
        /**
         * @brief Constructs a window with the given specification.
         * @param spec Configuration options for the window
         */
        Window(const WindowSpecification &spec);

        /**
         * @brief Destructor that ensures proper cleanup of window resources.
         */
        ~Window();

        /**
         * @brief Creates the GLFW window and initializes OpenGL context.
         *
         * This method creates the actual GLFW window, sets up the OpenGL context,
         * and initializes GLAD for OpenGL function loading.
         */
        void Create();

        /**
         * @brief Destroys the GLFW window and cleans up resources.
         *
         * This method destroys the GLFW window and releases associated resources.
         * Should be called before application termination.
         */
        void Destroy();

        /**
         * @brief Updates the window and swaps the front and back buffers.
         *
         * This method should be called at the end of each frame to present
         * the rendered content to the screen.
         */
        void Update();

        /**
         * @brief Gets the current framebuffer size in pixels.
         * @return 2D vector containing the framebuffer width and height
         *
         * The framebuffer size may differ from the window size on high-DPI displays.
         */
        glm::vec2 GetFrameBufferSize() const;

        /**
         * @brief Checks if the window should close.
         * @return True if the window close flag is set, false otherwise
         *
         * This typically happens when the user clicks the window's close button.
         */
        bool ShouldClose() const;

        /**
         * @brief Gets the underlying GLFW window handle.
         * @return Pointer to the GLFW window handle
         *
         * This provides direct access to the GLFW window for advanced operations.
         */
        GLFWwindow *GetHandle() const;

    private:
        WindowSpecification specification; ///< Window configuration
        GLFWwindow *handle;               ///< GLFW window handle
    };
} // namespace Core
