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

    # Add coverage targets if coverage is enabled
    if(ENABLE_COVERAGE)
        add_coverage_targets()
    endif()

    message(STATUS "Added comprehensive code quality targets:")
    message(STATUS "  - Individual: tidy-{core,engine,app,all}, format-{core,engine,app,all}")
    message(STATUS "  - Meta: code-quality, pre-commit-check")
    if(ENABLE_COVERAGE)
        message(STATUS "  - Coverage: coverage, coverage-html, coverage-report, coverage-clean")
    endif()
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

# ========================================
# Coverage Functions
# ========================================

# Function to add coverage targets
function(add_coverage_targets)
    if(NOT ENABLE_COVERAGE)
        return()
    endif()

    # Set coverage output directories
    set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
    set(COVERAGE_RAW_DIR "${COVERAGE_DIR}/raw")
    set(COVERAGE_HTML_DIR "${COVERAGE_DIR}/html")
    set(COVERAGE_DATA_FILE "${COVERAGE_DIR}/coverage.profdata")

    # Get test executable (assumes it's named TestVisionCraftNodes)
    set(TEST_EXECUTABLE "${CMAKE_BINARY_DIR}/tests/TestVisionCraftNodes${CMAKE_EXECUTABLE_SUFFIX}")

    # Get all source files for coverage (only src directory, not tests or external)
    file(GLOB_RECURSE COVERAGE_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
    )

    # Target: coverage-clean - Clean coverage data
    add_custom_target(coverage-clean
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${COVERAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/tests/default.profraw"
        COMMAND ${CMAKE_COMMAND} -E rm -f "default.profraw"
        COMMENT "Cleaning coverage data"
        VERBATIM
    )

    # Target: coverage-run - Run tests to generate coverage data
    add_custom_target(coverage-run
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_RAW_DIR}"
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${COVERAGE_RAW_DIR}/coverage-%p.profraw
                "${TEST_EXECUTABLE}"
        DEPENDS TestVisionCraftNodes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests to generate coverage data"
        VERBATIM
    )

    # Target: coverage-process - Process raw coverage data
    add_custom_target(coverage-process
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_DIR}"
        COMMAND ${LLVM_PROFDATA_EXE} merge
                -sparse
                "${COVERAGE_RAW_DIR}/coverage-*.profraw"
                -o "${COVERAGE_DATA_FILE}"
        DEPENDS coverage-run
        COMMENT "Processing coverage data with llvm-profdata"
        VERBATIM
    )

    # Target: coverage-report - Generate text coverage report
    add_custom_target(coverage-report
        COMMAND ${LLVM_COV_EXE} report
                "${TEST_EXECUTABLE}"
                -instr-profile="${COVERAGE_DATA_FILE}"
                ${COVERAGE_SOURCE_FILES}
        DEPENDS coverage-process
        COMMENT "Generating coverage report"
        VERBATIM
    )

    # Target: coverage-html - Generate HTML coverage report
    add_custom_target(coverage-html
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_HTML_DIR}"
        COMMAND ${LLVM_COV_EXE} show
                "${TEST_EXECUTABLE}"
                -instr-profile=${COVERAGE_DATA_FILE}
                -format=html
                -output-dir=${COVERAGE_HTML_DIR}
                -show-line-counts-or-regions
                -show-instantiation-summary
                ${COVERAGE_SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage HTML report generated: ${COVERAGE_HTML_DIR}/index.html"
        DEPENDS coverage-process
        COMMENT "Generating HTML coverage report"
        VERBATIM
    )

    # Target: coverage-summary - Generate summary with statistics
    add_custom_target(coverage-summary
        COMMAND ${LLVM_COV_EXE} report
                "${TEST_EXECUTABLE}"
                -instr-profile="${COVERAGE_DATA_FILE}"
                -show-region-summary=false
                ${COVERAGE_SOURCE_FILES}
        DEPENDS coverage-process
        COMMENT "Generating coverage summary"
        VERBATIM
    )

    # Target: coverage - Main coverage target (generates both reports)
    add_custom_target(coverage
        DEPENDS coverage-html coverage-report
        COMMENT "Generating all coverage reports"
    )

    # Target: coverage-open - Open HTML report in browser (Windows)
    if(WIN32)
        add_custom_target(coverage-open
            COMMAND cmd /c start "" "${COVERAGE_HTML_DIR}/index.html"
            DEPENDS coverage-html
            COMMENT "Opening coverage report in browser"
            VERBATIM
        )
    endif()

    message(STATUS "Added coverage targets: coverage, coverage-html, coverage-report, coverage-clean, coverage-open")
endfunction()
