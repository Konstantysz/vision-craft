cmake_minimum_required(VERSION 3.26)
project(vision_craft_app LANGUAGES CXX)

add_executable(VisionCraftApp
    src/main.cpp
)

target_compile_features(VisionCraftApp PRIVATE cxx_std_20)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(VisionCraftApp 
    PRIVATE 
    glad::glad 
    glfw 
    imgui::imgui
    spdlog::spdlog)

target_include_directories(VisionCraftApp PRIVATE
    ${imgui_SOURCE_DIR}
    #${imgui_SOURCE_DIR}/backends
    ${glad_SOURCE_DIR}/include
    ${glfw_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_definitions(VisionCraftApp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
