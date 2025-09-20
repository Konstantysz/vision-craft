#include "Window.h"

#include "Logger.h"

namespace Core
{
    Window::Window(const WindowSpecification &spec) : specification(spec)
    {
    }

    Window::~Window()
    {
        Destroy();
    }

    void Window::Create()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        handle =
            glfwCreateWindow(specification.width, specification.height, specification.title.c_str(), nullptr, nullptr);
        if (!handle)
        {
            LOG_ERROR("Failed to create GLFW window.");
            assert(false);
        }

        glfwMakeContextCurrent(handle);
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

        glfwSwapInterval(specification.vSync ? 1 : 0);
    }

    void Window::Destroy()
    {
        if (handle)
        {
            glfwDestroyWindow(handle);
        }

        handle = nullptr;
    }

    void Window::Update()
    {
        glfwSwapBuffers(handle);
    }

    glm::vec2 Window::GetFrameBufferSize() const
    {
        int width, height;
        glfwGetFramebufferSize(handle, &width, &height);
        return { width, height };
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(handle);
    }

    GLFWwindow *Window::GetHandle() const
    {
        return handle;
    }
} // namespace Core
