cmake_minimum_required(VERSION 3.26)
project(vision_craft_app LANGUAGES CXX)

add_executable(VisionCraftApp
    VisionCraftGUI.cpp
    VisionCraftApplication.cpp
    VisionCraftAppLayer.cpp
    Layers/DockSpaceLayer.cpp
    Layers/CanvasLayer.cpp
    Layers/NodeEditorLayer.cpp
    Layers/PropertyPanelLayer.cpp
    Layers/GraphExecutionLayer.cpp)

target_compile_features(VisionCraftApp PRIVATE cxx_std_20)

target_compile_definitions(VisionCraftApp PUBLIC 
    IMGUI_IMPL_OPENGL_LOADER_GLAD 
    IMGUI_ENABLE_DOCKING
)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(VisionCraftApp 
    PRIVATE 
    glad::glad 
    glfw 
    imgui::imgui
    Core
    VisionCraftEngine
)

target_include_directories(VisionCraftApp PRIVATE
    ${imgui_SOURCE_DIR}
    ${glad_SOURCE_DIR}/include
    ${glfw_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_definitions(VisionCraftApp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
