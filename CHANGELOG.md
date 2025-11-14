# Changelog

All notable changes to VisionCraft will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Window State Persistence - Window position, size, and maximized state saved between sessions
- Recent Files Menu - Track and quickly reopen up to 10 recently used graph files
- Default Docking Layout - Execution panel on top, Properties on right, Node Editor in center
- Event-Based File Tracking - FileOpenedEvent and LoadGraphFromFileEvent for file operations
- RecentFilesManager class for JSON-based recent files tracking
- DockingLayoutHelper for setting up default ImGui dock layout
- Persistence files ignored in git (window_state.json, recent_files.json, imgui.ini)
- Domain-driven four-library architecture with clean separation of concerns

### Changed

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

- Default docking layout now correctly places Execution panel at top of window
- Circular dependency between Nodes and UI layers (NodeFactory now in Nodes library)

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

[Unreleased]: https://github.com/Konstantysz/vision-craft/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/Konstantysz/vision-craft/releases/tag/v0.1.0
