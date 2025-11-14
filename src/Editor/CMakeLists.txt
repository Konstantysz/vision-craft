find_package(nlohmann_json CONFIG REQUIRED)

add_library(Editor STATIC
    Commands/CommandHistory.cpp
    Commands/NodeCommands.cpp
    Commands/ConnectionCommands.cpp
    State/SelectionManager.cpp
    State/ClipboardManager.cpp
    Persistence/RecentFilesManager.cpp
)

target_include_directories(Editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Editor PUBLIC
    Nodes
    nlohmann_json::nlohmann_json
)

set_target_properties(Editor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "VisionCraft/Libraries"
)
