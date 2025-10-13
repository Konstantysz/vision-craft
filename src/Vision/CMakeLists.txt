add_library(Vision STATIC
    Nodes/GrayscaleNode.cpp
    Nodes/ThresholdNode.cpp
    Nodes/CannyEdgeNode.cpp
    IO/ImageInputNode.cpp
    IO/ImageOutputNode.cpp
    IO/PreviewNode.cpp
    IO/Texture.cpp
)

target_include_directories(Vision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Vision PUBLIC
    Nodes
    opencv_core opencv_imgproc opencv_imgcodecs
    glad::glad
    glfw
)

set_target_properties(Vision PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft/Libraries"
)
