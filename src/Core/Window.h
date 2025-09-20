#pragma once
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Core
{
    struct WindowSpecification
    {
        std::string title;
        unsigned int width = 1280;
        unsigned int height = 720;
        bool isResizable = true;
        bool vSync = true;
    };

    class Window
    {
    public:
        Window(const WindowSpecification &spec);

        ~Window();

        void Create();

        void Destroy();

        void Update();

        glm::vec2 GetFrameBufferSize() const;

        bool ShouldClose() const;

        GLFWwindow *GetHandle() const;

    private:
        WindowSpecification specification;
        GLFWwindow *handle;
    };
}
