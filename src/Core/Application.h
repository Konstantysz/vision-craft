#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Layer.h"
#include "Window.h"

namespace Core
{
    struct ApplicationSpecification
    {
        std::string name = "Application";
        WindowSpecification windowSpecification;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification &specification = ApplicationSpecification());

        ~Application();

        void Run();

        void Stop();

        template<typename TLayer>
            requires std::is_base_of_v<Layer, TLayer>
        void PushLayer()
        {
            static_assert(std::is_default_constructible_v<TLayer>, "Layer must be default constructible");
            layerStack.push_back(std::make_unique<TLayer>());
        }

        glm::vec2 GetFramebufferSize() const;

        static Application &Get();

        static float GetTime();

    private:
        ApplicationSpecification specification;
        std::vector<std::unique_ptr<Layer>> layerStack;
        std::shared_ptr<Window> window;
        bool isRunning = false;
    };
} // namespace Core