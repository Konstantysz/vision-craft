# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VisionCraft is a modern C++20 computer vision node editor with an intuitive visual programming interface. The project uses CMake, vcpkg for dependencies, and follows a clean layered architecture with domain-driven design.

## Build and Development Commands

### Building

```bash
# First-time setup (with submodules)
git clone --recursive https://github.com/Konstantysz/vision-craft.git
cd vision-craft

# Configure build
cmake -B build

# Build (Release)
cmake --build build --config Release

# Build (Debug)
cmake --build build --config Debug

# Development build with static analysis
cmake -B build -DENABLE_CLANG_TIDY=ON -DENABLE_COMPILE_COMMANDS=ON
cmake --build build --config Release
```

### Running

```bash
# Windows
.\build\src\App\Release\VisionCraft.exe

# Linux
./build/src/App/VisionCraft
```

### Testing

```bash
# Run all tests
cd build
ctest --output-on-failure

# Run specific test
ctest -R TestNodeEditor --output-on-failure

# Build with coverage (requires LLVM tools)
cmake -B build -DENABLE_COVERAGE=ON
cmake --build build
ctest
# Coverage reports generated in build/coverage/
```

### Code Quality

```bash
# Install pre-commit hooks (one-time setup)
pip install pre-commit
pre-commit install

# Run all code quality checks
python scripts/run-code-quality.py

# Format only
python scripts/run-code-quality.py --format-only

# Static analysis only
python scripts/run-code-quality.py --tidy-only

# CMake targets
cmake --build build --target format-all    # Apply clang-format
cmake --build build --target tidy-all      # Run clang-tidy
cmake --build build --target code-quality  # All checks
```

## Architecture

VisionCraft follows a layered architecture with strict dependency direction (bottom to top):

```
Nodes (Core abstractions)
  ↓
Vision (CV-specific nodes)
  ↓
Editor (Commands, state management)
  ↓
UI (Rendering, canvas, layers)
  ↓
App (Application entry point)
```

### Domain Structure

**Nodes Domain** (`src/Nodes/Core/`):
- `Node` - Abstract base class for all nodes, defines `Process()` interface
- `NodeEditor` - Central graph management: execution engine, serialization, async execution
- `Slot` - Type-safe data containers with C++20 concepts (`ValidNodeDataType`)
- `NodeData` - Variant type (`std::variant`) for data flow between nodes
- Key insight: Slots support optional default values for disconnected inputs

**Vision Domain** (`src/Vision/`):
- Computer vision nodes organized by category:
  - `IO/` - ImageInput, ImageOutput, Preview (texture management)
  - `Algorithms/` - Grayscale, Threshold, Canny, Sobel, Morphology, etc.
- `Factory/NodeFactory` - Registration system using C++20 concepts
- All nodes registered in `RegisterAllNodes()` with type strings

**Editor Domain** (`src/Editor/`):
- Command pattern for undo/redo (`Commands/`)
  - Base `Command` interface: `Execute()`, `Undo()`, `GetDescription()`
  - Node commands: CreateNode, DeleteNode, MoveNodes
  - Connection commands: CreateConnection, DeleteConnection
  - `CommandHistory` - Linear undo stack (executing new command clears redo)
- State management (`State/`)
  - `SelectionManager` - Single/multi/box selection, drag state
  - `ClipboardManager` - Copy/cut/paste with connection remapping
- Persistence (`Persistence/`)
  - `RecentFilesManager` - Recent files list with JSON persistence
  - Graph serialization in `NodeEditor` (JSON format)

**UI Domain** (`src/UI/`):
- Canvas system (`Canvas/`)
  - `CanvasController` - Pan/zoom transformations, grid rendering, visibility culling
  - `ConnectionManager` - Interactive connection creation, Bezier rendering, hit testing
  - `InputHandler` - Mouse/keyboard input processing, shortcuts
- Rendering (`Rendering/`)
  - `NodeRenderer` - Main node rendering pipeline
  - `NodeDimensionCalculator` - Size calculation based on pins/params
  - Strategy pattern for custom rendering:
    - `NodeRenderingStrategy` interface
    - `DefaultNodeRenderingStrategy` - No custom content
    - `ImageInputNodeRenderingStrategy` - File path + preview
    - `PreviewNodeRenderingStrategy` - Pass-through image display
- Widgets (`Widgets/`)
  - `NodeEditorTypes.h` - Core structures (NodePosition, PinId, NodeConnection)
  - `NodeSearchPalette` - Quick node creation (Ctrl+Space)
  - `ContextMenuRenderer` - Right-click menus
  - `FileDialogManager` - File dialogs using ImGuiFileDialog
- Layer architecture (`Layers/`)
  - `DockSpaceLayer` - Menu bar, dockspace, file operations
  - `NodeEditorLayer` - Main canvas with nodes/connections
  - `PropertyPanelLayer` - Node property inspector (right dock)
  - `GraphExecutionLayer` - Execute button, progress bar (bottom dock)

**App Domain** (`src/App/`):
- `VisionCraftApplication` - Main application class
  - Inherits from `Kappa::Application` (framework from external/kappa-core submodule)
  - Initializes ImGui (OpenGL3 + GLFW backends)
  - Owns shared `NodeEditor` instance passed to relevant layers
  - Registers all node types on startup

### Key Design Patterns

1. **Command Pattern** (Editor domain): All user actions encapsulated as commands for undo/redo. Commands use functors (`std::function`) for operations rather than direct references, enabling testability.

2. **Factory Pattern** (Vision domain): `NodeFactory` decouples node creation from types. Uses C++20 concepts for type-safe registration. Type strings enable JSON deserialization.

3. **Strategy Pattern** (UI rendering): Node-specific rendering behaviors without polluting Node base class. `NodeRenderer` selects strategy based on node type.

4. **Observer Pattern** (Layer system): Kappa event system for layer communication. Custom events: `FileOpenedEvent`, `LoadGraphEvent`, `SaveGraphEvent`, `GraphExecuteEvent`.

5. **Template Method Pattern** (Node processing): `NodeEditor::Execute()` defines algorithm, delegates to `node->Process()`.

### Graph Execution Flow

1. User clicks "Execute" in `GraphExecutionLayer`
2. `NodeEditor::ExecuteAsync()` launches background thread with `std::stop_token` for cancellation
3. `TopologicalSort()` determines execution order using Kahn's algorithm (detects cycles)
4. For each node:
   - `PassDataBetweenNodes()` - Copy output slot data to connected input slots
   - `node->Process()` - Execute node logic
   - Progress callback updates UI
   - Check cancellation token
5. Results returned via `std::shared_future<bool>`

Thread safety: `NodeEditor` uses `std::recursive_mutex graphMutex` for all graph operations.

### Data Flow

- `NodeData` variant holds: `cv::Mat` (images), numeric types, `std::string`, `std::filesystem::path`, `std::vector<cv::Point>`
- Slots use `GetInputValue<T>()` → `std::optional<T>` for type-safe access
- `GetValueOrDefault<T>()` auto-falls back to slot default when disconnected
- OpenCV `cv::Mat` uses reference counting (zero-copy in slots)

### Connection Rules
- Output pin → Input pin only (enforced by `ConnectionManager::IsConnectionValid()`)
- Input pins limited to ONE connection (enforced by `RemoveConnectionToInput()`)
- Output pins can have MULTIPLE connections
- Type checking: Pin data types must match (or be compatible)
- **Execution Flow Rules**:
  - **1:1 Enforcement**: Execution pins (white arrows) strictly enforce 1:1 connections (one output to one input) at the core layer (`NodeEditor::AddConnection`).
  - **Cycle Prevention**: Execution flow must be acyclic. `BuildExecutionPlan` performs cycle detection.

### Execution Flow Architecture
- **Hybrid Model**: Separates control flow (execution pins) from data flow (data pins).
- **Execution Pins**: White arrows (`PinType::Execution`). Define order of operations.
- **Data Pins**: Colored circles (`PinType::Data`). Define data dependencies.
- **Execution Plan**:
  - `BuildExecutionPlan()` compiles the graph into a linear `std::vector<ExecutionStep>`.
  - Uses topological sort on execution connections.
  - Precomputes incoming data connection indices for O(1) access during execution.
- **Lookahead Advancement**: `ExecutionFrame` advances `nextInstructionIndex` *before* executing the current node.
- **Pin Separation**: Use `Widgets::SeparatePinsByType()` helper to separate execution/data pins in UI code.

### Serialization

JSON format (saved by `NodeEditor::SaveToFile()`):
```json
{
  "nodes": [
    {"id": 1, "type": "ImageInput", "name": "Input", "position": {"x": 100, "y": 200}}
  ],
  "connections": [
    {"from": 1, "to": 2}
  ]
}
```

Deserialization: Nodes recreated via `NodeFactory::CreateNode(type, id, name)`, then connections restored.

## Modern C++20 Features

The codebase extensively uses C++20:

- **Concepts**: `ValidNodeDataType`, `NodeType` for compile-time type safety
- **Spaceship operator (`<=>`)**: `PinId`, `NodeConnection` comparisons
- **std::span**: Non-owning views (e.g., `GetConnections()`)
- **Ranges** (`std::views::keys`): Extract map keys in `GetNodeIds()`
- **std::stop_token/std::stop_source**: Cooperative cancellation for async execution
- **std::shared_future**: Async results shared across threads
- **std::variant + std::visit**: Type-safe `NodeData` handling
- **std::optional**: Return types for nullable values
- **Designated initializers**: `{.from = from, .to = to}`

## Important File Locations

### Core Node System
- `src/Nodes/Core/Node.h` - Abstract node base class
- `src/Nodes/Core/NodeEditor.h` - Graph management, execution, serialization
- `src/Nodes/Core/Slot.h` - Type-safe data containers
- `src/Nodes/Core/NodeData.h` - Variant type definition

### Adding New Nodes
1. Create class inheriting `Nodes::Node` in appropriate domain (Vision/Algorithms or Vision/IO)
2. Implement `GetType()` returning unique type string
3. Implement `Process()` with node logic
4. Define slots in constructor: `CreateInputSlot("name", defaultValue)`, `CreateOutputSlot("name")`
5. Register in `Vision::NodeFactory::RegisterAllNodes()`:
   ```cpp
   NodeFactory::RegisterNode<MyNode>("MyNode");
   ```
6. Optional: Create custom rendering strategy if needed (inherit `NodeRenderingStrategy`)

### Adding New Commands
1. Inherit from `Editor::Command`
2. Implement `Execute()`, `Undo()`, `GetDescription()`
3. Store all necessary state in constructor (before `Execute()` called)
4. Use functors for operations to maintain decoupling
5. Integrate with `InputHandler` for keyboard shortcuts

### Adding New Data Types to NodeData
1. Add type to `std::variant` in `src/Nodes/Core/NodeData.h`
2. Add color mapping in `NodeRenderer::GetDataTypeColor()`
3. Add enum to `PinDataType` in `src/UI/Widgets/NodeEditorTypes.h`
4. Add input widget rendering in `NodeRenderer::RenderNodeParameters()` if needed

## Testing

Tests located in `tests/` directory:
- `TestNodes.cpp` - Node base class tests
- `TestSlot.cpp` - Slot operations (get/set/clear/defaults)
- `TestNodeEditor.cpp` - Graph management, execution, topological sort, cycle detection
- `TestNodeFactory.cpp` - Factory registration and creation
- `TestCommandHistory.cpp` - Undo/redo mechanics
- `TestNodeCommands.cpp` / `TestConnectionCommands.cpp` - Command implementations
- `TestImageNodes.cpp` / `TestFilterNodes.cpp` / `TestConversionNodes.cpp` - Vision nodes
- `TestNodeSearchPalette.cpp` - UI widget tests

Run specific test suite:
```bash
cd build
ctest -R TestNodeEditor --output-on-failure
```

## Code Standards

- **Language**: C++20 (set in CMakeLists.txt)
- **Formatting**: clang-format (`.clang-format` config)
  - Line width: 120 characters
  - Indentation: 4 spaces
- **Static Analysis**: clang-tidy (`.clang-tidy` config)
- **Compiler Warnings**: `/W4` (MSVC), `-Wall -Wextra -Wpedantic -Wconversion` (GCC/Clang)

Pre-commit hooks automatically enforce formatting and run checks.

## Development Principles

This codebase follows core software engineering principles:

### KISS (Keep It Simple, Stupid)
- Prefer straightforward implementations over clever solutions
- Each class has a single, clear responsibility (e.g., `NodeRenderer` renders, `NodeEditor` manages graph)
- Avoid unnecessary abstractions - only abstract when multiple implementations exist or are anticipated
- Example: `Slot` class is a simple wrapper around `NodeData` with optional default, not an elaborate type system

### YAGNI (You Aren't Gonna Need It)
- Don't implement features or abstractions until they're actually needed
- Avoid speculative generalization - extend when requirements emerge
- Example: Node system started with basic types (cv::Mat, primitives) and expands only when new use cases appear
- Resist adding "just in case" parameters or configuration options

### SOLID Principles

**Single Responsibility Principle (SRP)**:
- Each class has one reason to change
- `NodeEditor` manages graph structure/execution, not rendering
- `CanvasController` handles pan/zoom, not node selection
- `CommandHistory` manages undo/redo, not command creation

**Open/Closed Principle (OCP)**:
- Open for extension, closed for modification
- Add new nodes without modifying `NodeEditor` or `NodeFactory` logic
- Add new rendering behaviors via `NodeRenderingStrategy` without changing `NodeRenderer` core
- Add new commands without modifying `CommandHistory`

**Liskov Substitution Principle (LSP)**:
- Derived classes are substitutable for base classes
- All `Node` subclasses honor the `Process()` contract (read inputs, write outputs, no side effects beyond outputs)
- All `Command` implementations support `Execute()` and `Undo()` symmetrically

**Interface Segregation Principle (ISP)**:
- Clients shouldn't depend on interfaces they don't use
- `Node` interface is minimal - only `Process()` and `GetType()` required
- Layers only receive relevant events via `OnEvent()`, not entire application state
- Commands use functors for specific operations, not full `NodeEditor` interface

**Dependency Inversion Principle (DIP)**:
- Depend on abstractions, not concretions
- UI depends on `Node` interface, not specific node implementations
- Commands depend on functors (`std::function`), not concrete editor methods
- `NodeFactory` depends on `Node` interface, concrete nodes register themselves

### DRY (Don't Repeat Yourself)
- Extract common logic into reusable functions/classes
- Slot access templates (`GetInputValue<T>()`) eliminate per-type boilerplate
- Command pattern prevents duplicating undo/redo logic across operations
- Factory pattern eliminates node creation boilerplate
- When you find yourself copying code, extract it into a shared utility

**Anti-patterns to avoid**:
- Copy-pasting node implementations - extract common base or utility functions
- Duplicating validation logic - centralize in validators
- Repeating rendering code - use strategies or helper functions
- Multiple serialization implementations - use common serialization utilities

### Applying These Principles

When adding new features:
1. **Start simple** (KISS) - implement the minimum that works
2. **Don't anticipate** (YAGNI) - extend when you have concrete requirements
3. **Check responsibilities** (SRP) - if a class does multiple things, split it
4. **Use abstractions** (OCP/DIP) - depend on interfaces, extend via inheritance/composition
5. **Look for duplication** (DRY) - extract common patterns before they spread

When reviewing code:
- Can this be simpler? (KISS)
- Is this solving a current problem or a hypothetical one? (YAGNI)
- Does this class have multiple reasons to change? (SRP)
- Are we modifying existing code instead of extending? (OCP)
- Would changing this break substitutability? (LSP)
- Are we forcing dependencies on unused functionality? (ISP)
- Are we depending on concrete implementations? (DIP)
- Have I seen this logic elsewhere? (DRY)

## Common Issues and Solutions

### Issue: Node not appearing in search palette
**Solution**: Verify node is registered in `Vision::NodeFactory::RegisterAllNodes()` with correct type string matching `GetType()` implementation.

### Issue: Connection validation failing
**Solution**: Check rules in `ConnectionManager::IsConnectionValid()`:
- Must be output pin → input pin
- Data types must match (check pin type derivation in `GetPinDataType()`)
- Cannot create cycles (topological sort validates)

### Issue: Undo not working
**Solution**: Ensure command stores all state in constructor before `Execute()` runs. Check `CommandHistory` is receiving command via `ExecuteCommand()`, not direct `Execute()`.

### Issue: Data not flowing between nodes
**Solution**:
- Verify slot names match exactly between output and input
- Check `PassDataBetweenNodes()` in execution logs
- Ensure `Process()` calls `SetOutputSlotData()` with correct data type
- Verify no exceptions thrown in `node->Process()`

### Issue: Graph execution hangs
**Solution**: Check for cycles - `TopologicalSort()` returns empty vector if cycle detected. Enable execution logging to identify stuck node.

### Issue: Node rendering incorrectly
**Solution**:
- Check `CalculateExtraHeight()` if custom content needed
- Verify `NodeRenderingStrategy` created for node type in `NodeRenderer::CreateRenderingStrategy()`
- Inspect `NodeDimensionCalculator` for size calculation issues

## Dependencies

Managed via vcpkg (`vcpkg.json`):
- **GLFW** - Window management
- **GLAD** - OpenGL loading
- **ImGui** (docking-experimental, glfw-binding, opengl3-binding) - UI framework
- **GLM** - Math library
- **spdlog** - Logging
- **OpenCV** - Computer vision
- **nlohmann-json** - JSON serialization
- **Google Test** - Unit testing

External submodules:
- `external/kappa-core` - Application framework (Layer system, Window, Events)
- `external/ImGuiFileDialog` - File dialog library

## Build System Notes

- CMake 3.26+ required
- vcpkg automatically downloads dependencies on configure
- Build directory: `build/` (out-of-source only, in-source builds rejected)
- CMake options:
  - `ENABLE_CLANG_TIDY=ON` - Enable static analysis during build
  - `ENABLE_COMPILE_COMMANDS=ON` - Generate `compile_commands.json` (default ON)
  - `BUILD_TESTS=ON` - Build test suite (default ON)
  - `ENABLE_COVERAGE=ON` - Enable code coverage (requires LLVM tools)

Build order enforced in `src/CMakeLists.txt`:
```cmake
add_subdirectory(Nodes)    # Core abstractions
add_subdirectory(Vision)   # Depends on Nodes
add_subdirectory(Editor)   # Depends on Nodes
add_subdirectory(UI)       # Depends on Nodes, Vision, Editor
add_subdirectory(App)      # Depends on all above
```

## Kappa Framework Integration

VisionCraft uses the Kappa framework (git submodule in `external/kappa-core`):

- `Kappa::Application` - Main application class with window management
- `Kappa::Layer` - Base class for UI layers (provides `OnEvent`, `OnUpdate`, `OnRender`)
- `Kappa::Event` system - Event propagation between layers
- `Kappa::Window` - GLFW window abstraction

Layers pushed in `VisionCraftApplication` constructor are rendered bottom-to-top.

## Performance Considerations

- Slot value caching: Connected data cached in slot to avoid repeated lookups
- Visibility culling: `CanvasController::IsRectangleVisible()` skips off-screen nodes
- OpenCV `cv::Mat` reference counting: Slots store shallow copies (zero-copy)
- Strategy pattern caching: Rendering strategy created once per node instance
- Connection hit testing: Bezier curve approximation for click detection

## Future Extension Points

The architecture supports easy extension:

1. **New node types**: Follow pattern in `src/Vision/Algorithms/`, register in factory
2. **New data types**: Add to `NodeData` variant, update rendering/colors
3. **Custom rendering**: Create `NodeRenderingStrategy` subclass
4. **New commands**: Inherit `Command`, integrate with `InputHandler`
5. **New layers**: Inherit `Kappa::Layer`, push in application
6. **New UI panels**: Create dockable ImGui windows in layers

## Useful Patterns for Development

### Creating a New Algorithm Node

```cpp
// In src/Vision/Algorithms/MyNode.h
class MyNode : public Nodes::Node {
public:
    MyNode(NodeId id, const std::string& name);
    void Process() override;
    std::string GetType() const override { return "MyNode"; }
};

// In src/Vision/Algorithms/MyNode.cpp
MyNode::MyNode(NodeId id, const std::string& name) : Node(id, name) {
    CreateInputSlot("Input");           // cv::Mat input
    CreateInputSlot("Param", 1.0);      // double parameter with default
    CreateOutputSlot("Output");         // cv::Mat output
}

void MyNode::Process() {
    auto input = GetInputValue<cv::Mat>("Input");
    if (!input || input->empty()) {
        ClearOutputSlot("Output");
        return;
    }

    auto param = GetInputValue<double>("Param").value_or(1.0);

    cv::Mat output;
    // ... process image ...

    SetOutputSlotData("Output", output);
}

// Register in src/Vision/Factory/NodeFactory.cpp
void NodeFactory::RegisterAllNodes() {
    // ...
    RegisterNode<MyNode>("MyNode");
}
```

### Creating a Command

```cpp
// In src/Editor/Commands/MyCommand.h
class MyCommand : public Command {
public:
    MyCommand(/* capture state before execute */);
    void Execute() override;
    void Undo() override;
    std::string GetDescription() const override;
private:
    // Store state for undo
};

// Usage
auto cmd = std::make_unique<MyCommand>(/* state */);
commandHistory.ExecuteCommand(std::move(cmd));
```

### Accessing Node Editor State

```cpp
// In a layer with NodeEditor reference
for (auto nodeId : nodeEditor.GetNodeIds()) {
    Node* node = nodeEditor.GetNode(nodeId);
    // ... work with node ...
}

auto connections = nodeEditor.GetConnections();
for (const auto& conn : connections) {
    // conn.outputPin, conn.inputPin
}
```
