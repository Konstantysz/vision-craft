cmake_minimum_required(VERSION 3.26)
project(kappa_core_minimal_example LANGUAGES CXX)

# Add the Core library (adjust path as needed)
add_subdirectory(../.. Core)

# Create example executable
add_executable(MinimalExample main.cpp)

# Link against Core
target_link_libraries(MinimalExample PRIVATE Core)

# Set C++20
target_compile_features(MinimalExample PRIVATE cxx_std_20)
