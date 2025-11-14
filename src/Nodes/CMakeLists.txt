find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(Nodes STATIC
    Core/Node.cpp
    Core/NodeEditor.cpp
    Core/Slot.cpp
    Core/NodeData.h
)

target_include_directories(Nodes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Nodes PUBLIC
    Kappa
    opencv_core
    nlohmann_json::nlohmann_json
)

set_target_properties(Nodes PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft/Libraries"
)
