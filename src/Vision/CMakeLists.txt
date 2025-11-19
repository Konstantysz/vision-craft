find_package(OpenCV CONFIG REQUIRED)

add_library(Vision STATIC
    Algorithms/GrayscaleNode.cpp
    Algorithms/ThresholdNode.cpp
    Algorithms/CannyEdgeNode.cpp
    Algorithms/CvtColorNode.cpp
    Algorithms/GrayscaleNode.cpp
    Algorithms/MedianBlurNode.cpp
    Algorithms/MergeChannelsNode.cpp
    Algorithms/MorphologyNode.cpp
    Algorithms/ResizeNode.cpp
    Algorithms/SobelNode.cpp
    Algorithms/SplitChannelsNode.cpp
    IO/ImageInputNode.cpp
    IO/ImageOutputNode.cpp
    IO/PreviewNode.cpp
    Factory/NodeFactory.cpp
)

target_include_directories(Vision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Vision PUBLIC
    Nodes
    Kappa
    opencv_core opencv_imgproc opencv_imgcodecs
)

set_target_properties(Vision PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft/Libraries"
)
