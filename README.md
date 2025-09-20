# 🎨 VisionCraft

A modern C++20 computer vision node editor with an intuitive visual programming interface. VisionCraft allows users to create complex computer vision pipelines through a drag-and-drop node-based editor. 🚀

## ✨ Features

- **Visual Node Editor**: Intuitive drag-and-drop interface for building computer vision pipelines
- **Layered Architecture**: Clean, modular design
- **Real-time Processing**: Execute and preview computer vision operations in real-time
- **Docking Interface**: Flexible workspace with dockable panels
- **Cross-platform**: Supports Windows and Linux

## 🏛️ Architecture

VisionCraft is built using a layered architecture:

- **Core**: Foundation framework providing Application, Window, and Layer abstractions
- **VisionCraftEngine**: Node system and computer vision processing logic
- **VisionCraftApp**: GUI application with ImGui-based interface

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

## 🚀 Installation & Build

### 🪟 Windows

1. **Clone the repository**
   ```cmd
   git clone https://github.com/yourusername/vision-craft.git
   cd vision-craft
   ```

2. **Install vcpkg (if not already installed)**
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   cd ..
   ```

3. **Configure CMake with vcpkg**
   ```cmd
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

4. **Build the project**
   ```cmd
   cmake --build . --config Release
   ```

5. **Run VisionCraft**
   ```cmd
   .\src\VisionCraftApp\Release\VisionCraftApp.exe
   ```

### 🐧 Linux

1. **Install dependencies**

   **Ubuntu/Debian:**
   ```bash
   sudo apt update
   sudo apt install build-essential git cmake pkg-config
   sudo apt install libgl1-mesa-dev libglu1-mesa-dev
   ```

   **Fedora/RHEL:**
   ```bash
   sudo dnf install gcc-c++ git cmake pkgconfig
   sudo dnf install mesa-libGL-devel mesa-libGLU-devel
   ```

2. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/vision-craft.git
   cd vision-craft
   ```

3. **Install vcpkg (if not already installed)**
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ./vcpkg integrate install
   cd ..
   ```

4. **Configure CMake with vcpkg**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

5. **Build the project**
   ```bash
   cmake --build . --config Release -j$(nproc)
   ```

6. **Run VisionCraft**
   ```bash
   ./src/VisionCraftApp/VisionCraftApp
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
├── src/
│   ├── Core/                 # Core framework
│   │   ├── Application.h     # Abstract application base
│   │   ├── Window.h          # Window management
│   │   ├── Layer.h           # Layer system
│   │   └── Logger.h          # Logging utilities
│   ├── VisionCraftEngine/    # Node system and CV logic
│   │   ├── Node.h            # Abstract node base
│   │   └── NodeEditor.h      # Node graph management
│   └── VisionCraftApp/       # GUI application
│       ├── VisionCraftApplication.h  # Main application
│       └── Layers/           # UI layers
├── tests/                    # Unit tests
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

### 🧪 Running Tests

```bash
cd build
ctest --output-on-failure
```

### 📐 Code Style

The project uses clang-format for code formatting.

### 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Core framework architecture inspired by [The Cherno's Application Layer tutorial](https://www.youtube.com/watch?v=rUxZ5N77M5E)
- Built with [ImGui](https://github.com/ocornut/imgui) for the user interface
- Uses [OpenCV](https://opencv.org/) for computer vision operations
- Powered by [GLFW](https://www.glfw.org/) for window management
- Logging provided by [spdlog](https://github.com/gabime/spdlog)