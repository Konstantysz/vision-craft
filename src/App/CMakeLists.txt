add_executable(VisionCraft
    VisionCraftGUI.cpp
    VisionCraftApplication.cpp
)

target_include_directories(VisionCraft PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(VisionCraft PRIVATE
    UI
)

set_target_properties(VisionCraft PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft"
    OUTPUT_NAME "VisionCraft"
)
