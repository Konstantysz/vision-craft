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
     * @brief Configuration for creating a window.
     */
    struct WindowSpecification
    {
        std::string title;         ///< Window title displayed in the title bar
        unsigned int width = 1280; ///< Window width in pixels
        unsigned int height = 720; ///< Window height in pixels
        bool isResizable = true;   ///< Whether the window can be resized by the user
        bool vSync = true;         ///< Whether vertical sync is enabled
    };

    /**
     * @brief GLFW window wrapper with OpenGL context.
     */
    class Window
    {
    public:
        /**
         * @brief Constructs window.
         * @param spec Window configuration
         */
        explicit Window(const WindowSpecification &spec);

        /**
         * @brief Destructor that ensures proper cleanup of window resources.
         */
        ~Window();

        /**
         * @brief Creates the GLFW window and initializes OpenGL.
         */
        void Create();

        /**
         * @brief Destroys the window.
         */
        void Destroy();

        /**
         * @brief Updates the window and swaps buffers.
         */
        void Update();

        /**
         * @brief Returns the framebuffer size.
         * @return Framebuffer size
         */
        [[nodiscard]] glm::vec2 GetFrameBufferSize() const;

        /**
         * @brief Checks if the window should close.
         * @return True if should close
         */
        [[nodiscard]] bool ShouldClose() const;

        /**
         * @brief Returns the GLFW window handle.
         * @return GLFW window handle
         */
        [[nodiscard]] GLFWwindow *GetHandle() const;

    private:
        WindowSpecification specification; ///< Window configuration
        GLFWwindow *handle;                ///< GLFW window handle
    };
} // namespace Core
