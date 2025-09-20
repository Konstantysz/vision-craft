#include "Application.h"

#include <cassert>

#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>

#include "Logger.h"

namespace Core
{
    static Application *instance = nullptr;

    static void GLFWErrorCallback(int error, const char *description)
    {
        LOG_ERROR("[GLFW Error] ({}): {}.", error, description);
    }

    Application::Application(const ApplicationSpecification &spec) : specification(spec)
    {
        instance = this;

        glfwSetErrorCallback(GLFWErrorCallback);
        glfwInit();

        if (specification.windowSpecification.title.empty())
        {
            specification.windowSpecification.title = specification.name;
        }

        window = std::make_shared<Window>(specification.windowSpecification);
        window->Create();
    }

    Application::~Application()
    {
        window->Destroy();

        glfwTerminate();

        instance = nullptr;
    }

    void Application::Run()
    {
        isRunning = true;

        auto lastTime = GetTime();

        while (isRunning)
        {
            glfwPollEvents();

            if (window->ShouldClose())
            {
                Stop();
                break;
            }

            const auto currentTime = GetTime();
            const auto timestep = glm::clamp(currentTime - lastTime, 0.001f, 0.1f);
            lastTime = currentTime;

            for (auto &layer : layerStack)
            {
                layer->OnUpdate(timestep);
            }

            BeginFrame();

            for (auto &layer : layerStack)
            {
                layer->OnRender();
            }

            EndFrame();

            window->Update();
        }
    }

    void Application::Stop()
    {
        isRunning = false;
    }

    glm::vec2 Application::GetFramebufferSize() const
    {
        return window->GetFrameBufferSize();
    }

    Application &Application::Get()
    {
        assert(instance);
        return *instance;
    }

    float Application::GetTime()
    {
        return static_cast<float>(glfwGetTime());
    }
} // namespace Core
