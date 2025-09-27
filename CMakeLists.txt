cmake_minimum_required(VERSION 3.26)
project(vision_craft VERSION 0.1.0 LANGUAGES CXX)

cmake_policy(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not supported. Please create a build directory.")
endif()

# ========================================
# Code Quality Options
# ========================================

# Option to enable static analysis
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_COMPILE_COMMANDS "Generate compile_commands.json for tooling" ON)

# Generate compile_commands.json for clang-tidy and other tools
if(ENABLE_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Find clang-tidy if enabled
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE
        NAMES "clang-tidy"
        PATHS "C:/Program Files/LLVM/bin"
        DOC "Path to clang-tidy executable"
    )

    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

        # Configure clang-tidy command
        set(CLANG_TIDY_COMMAND
            "${CLANG_TIDY_EXE}"
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
            "--header-filter=${CMAKE_SOURCE_DIR}/src/.*"
            "--format-style=file"
        )

        # Set CMAKE_CXX_CLANG_TIDY to run on all targets
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})

        message(STATUS "clang-tidy integration enabled")
    else()
        message(WARNING "clang-tidy not found! Install LLVM or disable ENABLE_CLANG_TIDY")
    endif()
endif()

# ========================================
# Compiler Options
# ========================================

if(MSVC)
    add_compile_options(/W4 /permissive-)
    # Disable specific MSVC warnings that conflict with external libraries
    add_compile_options(/wd4251 /wd4275)  # DLL interface warnings
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

# ========================================
# Code Quality Integration
# ========================================

# Include code quality functions
include(cmake/CodeQuality.cmake)

# Setup code quality targets
add_code_quality_targets()

# Setup IDE integration
setup_ide_integration()

add_subdirectory(src)
add_subdirectory(tests)
