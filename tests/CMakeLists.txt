cmake_minimum_required(VERSION 3.26)

add_executable(TestVisionCraftNodes
    TestVisionCraftNodes.cpp
)

target_compile_features(TestVisionCraftNodes PRIVATE cxx_std_20)

target_include_directories(TestVisionCraftNodes PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(TestVisionCraftNodes
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    VisionCraftEngine
)

include(CTest)
enable_testing()
add_test(NAME TestVisionCraftNodes COMMAND TestVisionCraftNodes)