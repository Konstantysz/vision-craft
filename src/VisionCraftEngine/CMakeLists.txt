cmake_minimum_required(VERSION 3.26)
project(vision_craft_engine LANGUAGES CXX)

find_package(OpenCV CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(VisionCraftEngine STATIC
    Node.cpp
    NodeEditor.cpp
    LoadImageNode.cpp
    GrayscaleNode.cpp
)

target_include_directories(VisionCraftEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(VisionCraftEngine PUBLIC 
    opencv_core 
    opencv_imgcodecs 
    opencv_imgproc
    spdlog::spdlog)

target_compile_features(VisionCraftEngine PUBLIC cxx_std_20)
