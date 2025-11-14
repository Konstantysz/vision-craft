# 🎨 VisionCraft

[![Pre-commit](https://github.com/Konstantysz/vision-craft/actions/workflows/pre-commit.yml/badge.svg)](https://github.com/Konstantysz/vision-craft/actions/workflows/pre-commit.yml)

A modern C++20 computer vision node editor with an intuitive visual programming interface. VisionCraft allows users to create complex computer vision pipelines through a drag-and-drop node-based editor. 🚀

## ✨ Features

- **Visual Node Editor**: Intuitive drag-and-drop interface for building computer vision pipelines
- **Layered Architecture**: Clean, modular design
- **Real-time Processing**: Execute and preview computer vision operations in real-time
- **Docking Interface**: Flexible workspace with dockable panels
- **Cross-platform**: Supports Windows and Linux

## 🏛️ Architecture

VisionCraft is built using a domain-driven architecture with clear separation of concerns:

- **kappa-core**: Foundation framework (as git submodule) providing Application, Window, and Layer abstractions
- **Nodes**: Core node system abstractions (Node, NodeEditor, Slot, NodeData)
- **Vision**: Computer vision domain (algorithms, I/O nodes, node factory)
- **Editor**: Editor domain (commands, state management, persistence)
- **UI**: Presentation layer (rendering, canvas, widgets, application layers)
- **App**: Application composition and entry point

## 📋 Prerequisites

### 🌐 Common Requirements
- C++20 compatible compiler
- CMake 3.26 or higher
- vcpkg package manager

### 🪟 Windows
- Visual Studio 2022 (or Visual Studio Build Tools)
- Git for Windows

### 🐧 Linux
- GCC 11+ or Clang 13+
- Git
- Build essentials (`build-essential` on Ubuntu/Debian)

## 🚀 Quick Start

### 🪟 Windows

1. **Clone the repository with submodules**
   ```cmd
   git clone --recursive https://github.com/Konstantysz/vision-craft.git
   cd vision-craft
   ```

2. **Configure and build**
   ```cmd
   cmake -B build
   cmake --build build --config Release
   ```

3. **Run VisionCraft**
   ```cmd
   .\build\src\App\Release\VisionCraft.exe
   ```

### 🐧 Linux

1. **Install dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt install build-essential cmake libgl1-mesa-dev libglu1-mesa-dev

   # Fedora/RHEL
   sudo dnf install gcc-c++ cmake mesa-libGL-devel mesa-libGLU-devel
   ```

2. **Clone and build**
   ```bash
   git clone --recursive https://github.com/Konstantysz/vision-craft.git
   cd vision-craft
   cmake -B build
   cmake --build build --config Release -j$(nproc)
   ```

3. **Run VisionCraft**
   ```bash
   ./build/src/App/VisionCraft
   ```

## 🔧 Development Build

For development with code quality tools:

```bash
# Configure with static analysis
cmake -B build -DENABLE_CLANG_TIDY=ON -DENABLE_COMPILE_COMMANDS=ON
cmake --build build --config Release

# Run code quality checks
python scripts/run-code-quality.py
```

## 📚 Dependencies

VisionCraft automatically manages its dependencies through vcpkg:

- **GLFW**: Window management and input handling
- **GLAD**: OpenGL function loading
- **ImGui**: Immediate mode GUI framework
- **GLM**: Mathematics library for graphics
- **spdlog**: Fast C++ logging library
- **OpenCV**: Computer vision processing library
- **Google Test**: Unit testing framework

## 📁 Project Structure

```
vision-craft/
├── external/
│   ├── kappa-core/           # Core GUI application framework (git submodule)
│   └── ImGuiFileDialog/      # File dialog library
├── src/
│   ├── Nodes/                # Core node system abstractions
│   │   └── Core/             # Node, NodeEditor, Slot, NodeData
│   ├── Vision/               # Computer vision domain
│   │   ├── Algorithms/       # CV processing nodes (Grayscale, Threshold, CannyEdge)
│   │   ├── IO/               # Image I/O nodes (ImageInput, ImageOutput, Preview)
│   │   └── Factory/          # Node factory for registration
│   ├── Editor/               # Editor domain
│   │   ├── Commands/         # Command pattern for undo/redo
│   │   ├── State/            # Selection and clipboard managers
│   │   └── Persistence/      # Recent files, serialization
│   ├── UI/                   # Presentation layer
│   │   ├── Layers/           # Application layers (NodeEditor, DockSpace, etc.)
│   │   ├── Rendering/        # Node rendering and strategies
│   │   ├── Canvas/           # Canvas controller, connections, input handling
│   │   ├── Widgets/          # UI components, dialogs, constants
│   │   └── Events/           # Application events
│   └── App/                  # Application entry point
│       └── VisionCraftApplication  # Main executable
├── tests/                    # Unit tests
├── cmake/                    # Build system extensions
│   └── CodeQuality.cmake     # Code quality integration
├── scripts/                  # Development tools
│   └── run-code-quality.py   # Code quality runner
├── .pre-commit-config.yaml   # Pre-commit hooks configuration
├── .clang-format            # Code formatting rules
├── .clang-tidy              # Static analysis configuration
├── CMakeLists.txt           # Main build configuration
└── vcpkg.json              # Package dependencies
```

## 🎮 Usage

1. **Launch VisionCraft** using the executable from the build process
2. **Create nodes** by right-clicking in the canvas area
3. **Connect nodes** by dragging from output pins to input pins
4. **Edit properties** using the property panel when a node is selected
5. **Execute the graph** using the Run button in the menu bar
6. **View results** in the results panel

## 👨‍💻 Development

### 🛠️ Code Quality Tools

VisionCraft includes integrated code quality tools:

```bash
# Install pre-commit hooks (one-time setup)
pip install pre-commit
pre-commit install

# Run all code quality checks
python scripts/run-code-quality.py

# Format code only
python scripts/run-code-quality.py --format-only

# Static analysis only
python scripts/run-code-quality.py --tidy-only

# CMake targets for individual checks
cmake --build build --target format-all        # Apply formatting
cmake --build build --target tidy-all          # Run static analysis
cmake --build build --target code-quality      # All checks
```

### 🧪 Running Tests

```bash
cd build
ctest --output-on-failure
```

### 📐 Code Standards

- **Language**: C++20
- **Formatting**: clang-format (automatically applied via pre-commit hooks)
- **Static Analysis**: clang-tidy integration for modern C++ practices
- **Line Width**: 120 characters
- **Indentation**: 4 spaces

### 🔄 Development Workflow

1. **Setup development environment**:
   ```bash
   cmake -B build -DENABLE_CLANG_TIDY=ON
   pip install pre-commit && pre-commit install
   ```

2. **Make changes**: Edit code following project standards

3. **Test locally**:
   ```bash
   cmake --build build
   python scripts/run-code-quality.py
   ```

4. **Commit**: Pre-commit hooks automatically run formatting and checks

### 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes following the code standards
4. Add tests for new functionality
5. Ensure all code quality checks pass (`python scripts/run-code-quality.py`)
6. Commit your changes (pre-commit hooks will run automatically)
7. Push to your branch (`git push origin feature/amazing-feature`)
8. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Core framework architecture inspired by [The Cherno's Application Layer tutorial](https://www.youtube.com/watch?v=rUxZ5N77M5E)
- Built with [ImGui](https://github.com/ocornut/imgui) for the user interface
- Uses [OpenCV](https://opencv.org/) for computer vision operations
- Powered by [GLFW](https://www.glfw.org/) for window management
- Logging provided by [spdlog](https://github.com/gabime/spdlog)
