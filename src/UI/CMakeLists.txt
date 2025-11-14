find_package(imgui CONFIG REQUIRED)

add_library(UI STATIC
    Layers/DockSpaceLayer.cpp
    Layers/NodeEditorLayer.cpp
    Layers/PropertyPanelLayer.cpp
    Layers/GraphExecutionLayer.cpp
    Rendering/NodeRenderer.cpp
    Rendering/NodeDimensionCalculator.cpp
    Rendering/Strategies/DefaultNodeRenderingStrategy.cpp
    Rendering/Strategies/ImageInputNodeRenderingStrategy.cpp
    Rendering/Strategies/PreviewNodeRenderingStrategy.cpp
    Canvas/CanvasController.cpp
    Canvas/ConnectionManager.cpp
    Canvas/InputHandler.cpp
    Widgets/DockingLayoutHelper.cpp
    Widgets/ContextMenuRenderer.cpp
    Widgets/FileDialogManager.cpp
    ${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_include_directories(UI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/external/ImGuiFileDialog
)

target_link_libraries(UI PUBLIC
    Editor
    Vision
    Nodes
    Kappa
    imgui::imgui
)

set_target_properties(UI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft/Libraries"
)
