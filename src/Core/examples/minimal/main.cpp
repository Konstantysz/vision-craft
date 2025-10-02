#include "Application.h"
#include "Layer.h"
#include "Logger.h"

#include <glad/glad.h>

/**
 * @brief Minimal example layer demonstrating basic rendering.
 */
class ExampleLayer : public Core::Layer
{
public:
    void OnUpdate(float deltaTime) override
    {
        // Could add game logic here
        frameCount++;

        if (frameCount % 60 == 0)
        {
            LOG_INFO("Running at ~60 FPS, frame {}", frameCount);
        }
    }

    void OnRender() override
    {
        // Clear screen with a nice color
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Your rendering code would go here
    }

private:
    int frameCount = 0;
};

int main()
{
    // Configure application
    Core::ApplicationSpecification spec;
    spec.name = "kappa-core Minimal Example";
    spec.width = 1280;
    spec.height = 720;

    LOG_INFO("Starting {} ({}x{})", spec.name, spec.width, spec.height);

    // Create application
    Core::Application app(spec);

    // Add our example layer
    app.PushLayer(std::make_shared<ExampleLayer>());

    LOG_INFO("Application initialized, entering main loop");

    // Run main loop
    app.Run();

    LOG_INFO("Application shutdown complete");

    return 0;
}
