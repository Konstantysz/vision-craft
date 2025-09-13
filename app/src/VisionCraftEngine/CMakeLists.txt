cmake_minimum_required(VERSION 3.26)
project(vision_craft_engine LANGUAGES CXX)

add_library(VisionCraftEngine STATIC
    Node.cpp
    NodeEditor.cpp
)

target_include_directories(VisionCraftEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(VisionCraftEngine PUBLIC cxx_std_20)
