# kappa-core

A modern C++20 application framework for OpenGL applications, providing essential infrastructure for window management, rendering, logging, and event handling.

## Features

- **Application Framework** - Complete application lifecycle management with layer-based architecture
- **Window Management** - GLFW-based window wrapper with framebuffer callbacks and OpenGL context
- **Texture System** - RAII-based OpenGL texture management
- **Logging** - Powerful spdlog-based logging system with source location tracking
- **Event System** - Type-safe event bus for decoupled communication
- **Layer Architecture** - Composable layer system for organizing rendering and update logic

## Components

### Core Components

- **`Application`** - Main application class managing window, layers, and render loop
- **`Window`** - GLFW window wrapper with OpenGL context management
- **`Layer`** - Abstract base class for application layers (UI, rendering, etc.)
- **`EventBus`** - Type-safe publish-subscribe event system
- **`Logger`** - Singleton logging system with multiple log levels
- **`Texture`** - RAII OpenGL texture wrapper

## Dependencies

- **GLFW 3** - Window and input management
- **glad** - OpenGL loader
- **glm** - OpenGL Mathematics library
- **spdlog** - Fast C++ logging library

All dependencies are managed via [vcpkg](https://vcpkg.io/).

## Requirements

- C++20 compatible compiler
- CMake 3.26+
- vcpkg (for dependency management)

## Installation

### Using as a Git Submodule

```bash
cd your-project
git submodule add https://github.com/yourusername/kappa-core.git external/kappa-core
git submodule update --init --recursive
```

### CMake Integration

```cmake
# Add kappa-core subdirectory
add_subdirectory(external/kappa-core)

# Link against Core library
target_link_libraries(YourApp PRIVATE Core)
```

## Quick Start

### Minimal Application

```cpp
#include "Application.h"
#include "Layer.h"

class MyLayer : public Core::Layer
{
public:
    void OnUpdate(float deltaTime) override
    {
        // Update logic here
    }

    void OnRender() override
    {
        // Rendering logic here
    }
};

int main()
{
    Core::ApplicationSpecification spec;
    spec.name = "My Application";
    spec.width = 1280;
    spec.height = 720;

    Core::Application app(spec);
    app.PushLayer(std::make_shared<MyLayer>());
    app.Run();

    return 0;
}
```

### Logging

```cpp
#include "Logger.h"

// Simple logging
LOG_INFO("Application started");
LOG_WARN("Warning message");
LOG_ERROR("Error occurred: {}", errorCode);

// Logging with source location
LOG_DEBUG("Debug info at {}:{}", __FILE__, __LINE__);
```

### Event System

```cpp
#include "EventBus.h"
#include "Event.h"

// Define custom event
struct MyEvent : public Core::Event
{
    int data;
};

// Subscribe to events
auto& eventBus = app.GetEventBus();
eventBus.Subscribe<MyEvent>([](const MyEvent& event) {
    LOG_INFO("Received event with data: {}", event.data);
});

// Publish events
eventBus.Publish(MyEvent{42});
```

### Texture Loading

```cpp
#include "Texture.h"

// Load texture from file (requires image loading library)
Core::Texture texture;
// ... load image data ...
texture.Create(width, height, imageData);

// Use texture
glBindTexture(GL_TEXTURE_2D, texture.GetID());
```

## Architecture

### Application Lifecycle

```
Application::Run()
├── Initialize Window
├── Load Resources
└── Main Loop
    ├── Process Events
    ├── Update Layers (OnUpdate)
    ├── Render Layers (OnRender)
    └── Swap Buffers
```

### Layer System

Layers are processed in the order they're added:
1. **Update Phase** - Each layer's `OnUpdate()` is called
2. **Render Phase** - Each layer's `OnRender()` is called

Layers can be used for:
- UI rendering (ImGui, etc.)
- Game world rendering
- Debug overlays
- Post-processing effects

## Examples

See the `examples/` directory for complete sample applications:
- `minimal` - Bare minimum application
- `logging` - Logging system demonstration
- `layers` - Multi-layer rendering
- `events` - Event bus usage

## Building

```bash
# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Run tests (if available)
ctest --test-dir build
```

## Project Structure

```
kappa-core/
├── Application.h/cpp    # Main application framework
├── Window.h/cpp         # GLFW window wrapper
├── Layer.h              # Layer base class
├── EventBus.h           # Event system
├── Logger.h             # Logging system
├── Texture.h/cpp        # OpenGL texture wrapper
├── Event.h              # Event base class
├── CMakeLists.txt       # Build configuration
├── README.md            # This file
└── examples/            # Example applications
```

## Design Philosophy

- **Modern C++** - Leverages C++20 features for clean, expressive code
- **RAII** - Resource management through constructors/destructors
- **Minimal Dependencies** - Only essential libraries, all via vcpkg
- **Type Safety** - Strong typing, compile-time checks where possible
- **Composability** - Layer system allows mixing different concerns
- **Zero-Cost Abstractions** - Thin wrappers with minimal overhead

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

MIT License - see LICENSE file for details

## Acknowledgments

Built with:
- [GLFW](https://www.glfw.org/) - Window and input
- [glad](https://glad.dav1d.de/) - OpenGL loader
- [glm](https://github.com/g-truc/glm) - Mathematics
- [spdlog](https://github.com/gabime/spdlog) - Logging

## Version History

### v1.0.0 (Initial Release)
- Application framework with layer system
- Window management with GLFW
- Logging system with spdlog
- Event bus for type-safe events
- OpenGL texture wrapper

---

Made with ❤️ for building modern C++ applications
