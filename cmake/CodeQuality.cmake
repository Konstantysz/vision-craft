# ========================================
# Code Quality CMake Functions
# ========================================

# Function to add clang-tidy target for a specific directory
function(add_clang_tidy_target TARGET_NAME SOURCE_DIR)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" PATHS "C:/Program Files/LLVM/bin")

    if(CLANG_TIDY_EXE)
        # Get all C++ source files
        file(GLOB_RECURSE SOURCE_FILES
            "${SOURCE_DIR}/*.cpp"
            "${SOURCE_DIR}/*.hpp"
            "${SOURCE_DIR}/*.c"
            "${SOURCE_DIR}/*.h"
            "${SOURCE_DIR}/*.cc"
            "${SOURCE_DIR}/*.cxx"
        )

        # Create clang-tidy target
        add_custom_target(${TARGET_NAME}
            COMMAND ${CLANG_TIDY_EXE}
                --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
                --header-filter=${CMAKE_SOURCE_DIR}/src/.*
                --format-style=file
                -p ${CMAKE_BINARY_DIR}
                ${SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on ${SOURCE_DIR}"
            VERBATIM
        )

        message(STATUS "Added clang-tidy target: ${TARGET_NAME}")
    else()
        message(WARNING "clang-tidy not found, cannot create target: ${TARGET_NAME}")
    endif()
endfunction()

# Function to add clang-format target
function(add_clang_format_target TARGET_NAME SOURCE_DIR)
    find_program(CLANG_FORMAT_EXE NAMES "clang-format" PATHS "C:/Program Files/LLVM/bin")

    if(CLANG_FORMAT_EXE)
        # Get all C++ source files
        file(GLOB_RECURSE SOURCE_FILES
            "${SOURCE_DIR}/*.cpp"
            "${SOURCE_DIR}/*.hpp"
            "${SOURCE_DIR}/*.c"
            "${SOURCE_DIR}/*.h"
            "${SOURCE_DIR}/*.cc"
            "${SOURCE_DIR}/*.cxx"
        )

        # Create format target
        add_custom_target(${TARGET_NAME}
            COMMAND ${CLANG_FORMAT_EXE}
                -i
                -style=file
                ${SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting code with clang-format in ${SOURCE_DIR}"
            VERBATIM
        )

        # Create format-check target (doesn't modify files)
        add_custom_target(${TARGET_NAME}-check
            COMMAND ${CLANG_FORMAT_EXE}
                --dry-run
                --Werror
                -style=file
                ${SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Checking code format with clang-format in ${SOURCE_DIR}"
            VERBATIM
        )

        message(STATUS "Added clang-format targets: ${TARGET_NAME}, ${TARGET_NAME}-check")
    else()
        message(WARNING "clang-format not found, cannot create target: ${TARGET_NAME}")
    endif()
endfunction()

# Function to add comprehensive code quality target
function(add_code_quality_targets)
    # Individual targets for different components
    add_clang_tidy_target(tidy-core "${CMAKE_SOURCE_DIR}/src/Core")
    add_clang_tidy_target(tidy-engine "${CMAKE_SOURCE_DIR}/src/VisionCraftEngine")
    add_clang_tidy_target(tidy-app "${CMAKE_SOURCE_DIR}/src/VisionCraftApp")
    add_clang_tidy_target(tidy-all "${CMAKE_SOURCE_DIR}/src")

    add_clang_format_target(format-core "${CMAKE_SOURCE_DIR}/src/Core")
    add_clang_format_target(format-engine "${CMAKE_SOURCE_DIR}/src/VisionCraftEngine")
    add_clang_format_target(format-app "${CMAKE_SOURCE_DIR}/src/VisionCraftApp")
    add_clang_format_target(format-all "${CMAKE_SOURCE_DIR}/src")

    # Meta target for all code quality checks
    add_custom_target(code-quality
        DEPENDS tidy-all format-all-check
        COMMENT "Running all code quality checks"
    )

    # Pre-commit simulation target
    add_custom_target(pre-commit-check
        COMMAND ${CMAKE_COMMAND} -E echo "Running pre-commit style checks..."
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target format-all-check
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tidy-all
        COMMENT "Simulating pre-commit checks"
    )

    message(STATUS "Added comprehensive code quality targets:")
    message(STATUS "  - Individual: tidy-{core,engine,app,all}, format-{core,engine,app,all}")
    message(STATUS "  - Meta: code-quality, pre-commit-check")
endfunction()

# Function to setup IDE integration
function(setup_ide_integration)
    # For Visual Studio Code
    if(EXISTS "${CMAKE_SOURCE_DIR}/.vscode")
        configure_file(
            "${CMAKE_SOURCE_DIR}/cmake/vscode-tasks.json.in"
            "${CMAKE_SOURCE_DIR}/.vscode/tasks.json"
            @ONLY
        )
        message(STATUS "Configured VS Code tasks")
    endif()

    # For CLion/Visual Studio - add compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)
endfunction()

# Function to add static analysis options to a target
function(add_static_analysis_to_target TARGET_NAME)
    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
        set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
        message(STATUS "Added clang-tidy to target: ${TARGET_NAME}")
    endif()
endfunction()
