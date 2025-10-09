cmake_minimum_required(VERSION 3.26)
project(vision_craft_app LANGUAGES CXX)

# ========================================
# Source Files (Organized by Feature)
# ========================================

set(MAIN_SOURCES
    Main/VisionCraftGUI.cpp
    Main/VisionCraftApplication.cpp
)

set(EDITOR_SOURCES
    Editor/NodeFactory.cpp
    Editor/ClipboardManager.cpp
)

set(RENDERING_SOURCES
    Rendering/NodeRenderer.cpp
    Rendering/NodeDimensionCalculator.cpp
    Rendering/Strategies/DefaultNodeRenderingStrategy.cpp
    Rendering/Strategies/ImageInputNodeRenderingStrategy.cpp
    Rendering/Strategies/PreviewNodeRenderingStrategy.cpp
)

set(CANVAS_SOURCES
    Canvas/CanvasController.cpp
)

set(CONNECTION_SOURCES
    Connections/ConnectionManager.cpp
)

set(INPUT_SOURCES
    Input/InputHandler.cpp
    Input/SelectionManager.cpp
)

set(UI_SOURCES
    UI/ContextMenuRenderer.cpp
    UI/FileDialogManager.cpp
)

set(LAYER_SOURCES
    Layers/DockSpaceLayer.cpp
    Layers/NodeEditorLayer.cpp
    Layers/PropertyPanelLayer.cpp
    Layers/GraphExecutionLayer.cpp
)

set(EXTERNAL_SOURCES
    ${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${EDITOR_SOURCES}
    ${RENDERING_SOURCES}
    ${CANVAS_SOURCES}
    ${CONNECTION_SOURCES}
    ${INPUT_SOURCES}
    ${UI_SOURCES}
    ${LAYER_SOURCES}
    ${EXTERNAL_SOURCES}
)

add_executable(VisionCraftApp ${ALL_SOURCES})

target_compile_features(VisionCraftApp PRIVATE cxx_std_20)

# ========================================
# Compile Definitions
# ========================================

target_compile_definitions(VisionCraftApp PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    IMGUI_ENABLE_DOCKING
    USE_STD_FILESYSTEM
)

# ========================================
# Dependencies
# ========================================

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(VisionCraftApp
    PRIVATE
        glad::glad
        glfw
        imgui::imgui
        Kappa
        VisionCraftEngine
)

# ========================================
# Include Directories
# ========================================

target_include_directories(VisionCraftApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${glad_SOURCE_DIR}/include
    ${glfw_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog
)

# ========================================
# Platform-Specific Settings
# ========================================

if(MSVC)
    target_compile_definitions(VisionCraftApp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()