cmake_minimum_required(VERSION 3.26)
project(core LANGUAGES CXX)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(Core STATIC
    Application.cpp
    Window.cpp
    Texture.cpp)

target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(Core PUBLIC
    glad::glad
    glfw
    glm::glm
    spdlog::spdlog)

target_compile_features(Core PUBLIC cxx_std_20)
