# Changelog

All notable changes to VisionCraft will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2025-11-16

### Added

- **C++20 Modern Features** (PR #10) - Comprehensive adoption of C++20 language features
  - C++20 Concepts for compile-time type safety (`ValidNodeDataType`, `NodeType`)
  - Spaceship operator (`operator<=>`) replacing 16 lines of comparison boilerplate
  - Designated initializers for explicit struct initialization
  - Ranges library (`std::views::keys`, `std::erase()`)
  - `std::span` for modern non-owning view APIs
  - Modern `contains()` method replacing `count() > 0` idiom
- **Editor Library Test Suite** (PR #3) - 91 comprehensive tests for command system
  - `TestCommandHistory.cpp` - 41 tests for undo/redo system validation
  - `TestNodeCommands.cpp` - 30 tests for node manipulation commands (Create, Delete, Move)
  - `TestConnectionCommands.cpp` - 20 tests for connection management commands
- **GitHub Actions CI/CD Workflows** - Elegant kappa-core-inspired separation of concerns
  - `build.yml` - Multi-platform build matrix (Windows MSVC, Linux GCC) with artifact upload
  - `tests.yml` - Automated testing triggered after successful builds (workflow_run pattern)
  - `coverage.yml` - LLVM coverage analysis (Windows Clang, 30-day artifact retention)
  - `static-analysis.yml` - clang-tidy-18 + cppcheck with full project build for accurate analysis
  - `format.yml` - clang-format-14 code formatting validation
  - vcpkg binary and package caching (reduces build time from ~40min to ~2-5min)
  - Build artifacts shared across workflows using dawidd6/action-download-artifact@v3
- **Automated Coverage Reporting** - PR comments with detailed coverage statistics
  - Extract coverage metrics from llvm-cov (Lines, Functions, Regions percentages)
  - Post formatted table to PR comments via github-script action
  - Link to full HTML coverage report in artifacts
- **Build Status Badges** - Windows and Linux build/test status in README.md
- **Coverage Infrastructure**
  - LLVM coverage with Clang compiler on Windows (Ninja generator required)
  - Python coverage script with Unicode symbol fixes for Windows console
  - Automated coverage generation in CI/CD pipeline
- Window State Persistence - Window position, size, and maximized state saved between sessions
- Recent Files Menu - Track and quickly reopen up to 10 recently used graph files
- Default Docking Layout - Execution panel on top, Properties on right, Node Editor in center
- Event-Based File Tracking - FileOpenedEvent and LoadGraphFromFileEvent for file operations
- RecentFilesManager class for JSON-based recent files tracking
- DockingLayoutHelper for setting up default ImGui dock layout
- Persistence files ignored in git (window_state.json, recent_files.json, imgui.ini)
- Domain-driven four-library architecture with clean separation of concerns

### Changed

- **Static Analysis Configuration** - Enhanced code quality tooling
  - Upgraded to clang-tidy-18 for better C++20 stdlib support and GCC 14 compatibility
  - clang-tidy now uses compile_commands.json from full project build (includes OpenCV paths)
  - cppcheck configured with proper include paths (`-I src`, `-I external/kappa-core/include`)
  - Both tools use continue-on-error + separate validation step (analyze all files, fail on errors)
  - System headers excluded from analysis (`--system-headers=false`, `-isystem` flags)
- **Build Optimization** - kappa-core submodule configured with BUILD_EXAMPLES=OFF, BUILD_TESTS=OFF (~30% faster builds)
- **BREAKING**: Restructured entire codebase to domain-driven architecture
  - Split monolithic VisionCraftEngine/VisionCraftApp into 4 focused libraries:
    - `Nodes` - Core node graph system (Node, NodeEditor, Slot, NodeData, NodeFactory)
    - `Vision` - Computer vision algorithms and I/O operations
    - `Editor` - Business logic (commands, state management, persistence)
    - `UI` - Presentation layer (ImGui layers, rendering, widgets, events)
  - Moved NodeFactory from UI layer to Nodes library (resolves circular dependency)
  - Reorganized Vision nodes: Algorithms/ (GrayscaleNode, CannyEdgeNode, ThresholdNode) and IO/ (ImageInputNode, ImageOutputNode, PreviewNode)
  - Updated all namespaces from `VisionCraft::Engine` to domain-specific namespaces:
    - `VisionCraft::Nodes`
    - `VisionCraft::Vision::{Algorithms,IO}`
    - `VisionCraft::Editor::{Commands,State,Persistence}`
    - `VisionCraft::UI::{Layers,Rendering,Canvas,Widgets,Events}`
    - `VisionCraft::App`
  - Updated all include paths to reflect new directory structure
  - Updated all CMakeLists.txt files with proper library dependencies
- Updated kappa-core dependency to v0.4.0
- DockSpaceLayer now includes Recent Files submenu in File menu
- NodeEditorLayer emits file events on successful save/load operations

### Fixed

- **Coverage Workflow** (PR #10) - Fixed test build and coverage generation
  - BUILD_TESTS variable was being force-disabled by kappa-core configuration
  - CMakeLists.txt now saves/restores BUILD_TESTS around kappa-core subdirectory
  - Added explicit `-DBUILD_TESTS=ON` flag to coverage workflow
  - Removed `continue-on-error` flags to properly fail on coverage errors
  - Simplified conditional logic from `if: always()` to `if: success()`
- **Cppcheck C++20 Support** (PR #10) - Resolved concept syntax parsing error
  - Reformatted `ValidNodeDataType` concept to single line with inline suppression
  - Added `// cppcheck-suppress internalAstError` for C++20 requires expression
- **Code Quality Issues** - Resolved cppcheck warnings
  - ConnectionManager constructor now explicitly initializes `connectionState` member
  - NodeEditorLayer::DeleteNode now handles `RemoveNode()` return value with warning log
  - Lambda in CreateNodeCommand uses `[[maybe_unused]]` for nodiscard compliance (modern C++)
- **CI/CD Static Analysis** - Fixed clang-tidy and cppcheck infrastructure issues
  - Added build step before clang-tidy to ensure all dependencies (OpenCV) are available
  - Removed manual include path overrides; clang-tidy now uses compile_commands.json exclusively
  - Fixed clang-tidy C++ standard library compatibility (clang-tidy-14 → clang-tidy-18)
  - Added vcpkg caching and proper dependency installation order in static analysis workflow
- Default docking layout now correctly places Execution panel at top of window
- Circular dependency between Nodes and UI layers (NodeFactory now in Nodes library)
- GitHub Actions workflow permissions for PR comments (`pull-requests: write`)
- Coverage job using correct compiler (Clang instead of MSVC for LLVM coverage)
- Ninja generator configuration for Windows Clang builds (removed incompatible `--config` flag)
- Unicode symbol encoding in Python coverage script for Windows console
- LLVM_PROFILE_FILE environment variable setup in CMake coverage targets

## [0.1.0] - 2025-10-06

### Added

- CMake build system with vcpkg
- Cross-platform support (Windows, Linux)
- MIT License
- Basic node editor with pan and zoom
- Image Input, Output, Preview nodes
- Grayscale, Canny Edge, Threshold processing nodes
- Save/load graph functionality
- File browser for image loading

---

[Unreleased]: https://github.com/Konstantysz/vision-craft/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/Konstantysz/vision-craft/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/Konstantysz/vision-craft/releases/tag/v0.1.0
