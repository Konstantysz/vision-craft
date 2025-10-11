cmake_minimum_required(VERSION 3.26)
project(vision_craft_engine LANGUAGES CXX)

find_package(OpenCV CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(VisionCraftEngine STATIC
    Node.cpp
    NodeEditor.cpp
    NodeFactory.cpp
    Slot.cpp
    Nodes/GrayscaleNode.cpp
    Nodes/ImageInputNode.cpp
    Nodes/CannyEdgeNode.cpp
    Nodes/ThresholdNode.cpp
    Nodes/ImageOutputNode.cpp
    Nodes/PreviewNode.cpp
)

target_include_directories(VisionCraftEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(VisionCraftEngine PUBLIC
    opencv_core
    opencv_imgcodecs
    opencv_imgproc)

target_link_libraries(VisionCraftEngine PUBLIC
    Kappa
    nlohmann_json::nlohmann_json)

target_compile_features(VisionCraftEngine PUBLIC cxx_std_20)
