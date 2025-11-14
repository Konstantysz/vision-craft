#include "NodeEditorLayer.h"
#include "Editor/Commands/ConnectionCommands.h"
#include "Editor/Commands/NodeCommands.h"
#include "UI/Events/FileOpenedEvent.h"
#include "UI/Events/LoadGraphEvent.h"
#include "UI/Events/NewGraphEvent.h"
#include "UI/Events/SaveGraphEvent.h"
#include "UI/Rendering/NodeRenderer.h"
#include "UI/Widgets/NodeEditorConstants.h"
#include "Logger.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <limits>

#include <imgui.h>

#include "Application.h"

#include "Vision/IO/ImageInputNode.h"


namespace VisionCraft::UI::Layers
{
    NodeEditorLayer::NodeEditorLayer(Nodes::NodeEditor &nodeEditor)
        : nodeEditor(nodeEditor), nodeRenderer(canvas, connectionManager),
          inputHandler(selectionManager, contextMenuRenderer, canvas)
    {
        // Register all available node types with the factory
        Vision::NodeFactory::RegisterAllNodes();

        // Register node types for context menu
        contextMenuRenderer.SetAvailableNodeTypes({
            { "ImageInput", "Image Input", "Input/Output" },
            { "ImageOutput", "Image Output", "Input/Output" },
            { "Preview", "Preview", "Input/Output" },
            { "Grayscale", "Grayscale", "Processing" },
            { "CannyEdge", "Canny Edge Detection", "Processing" },
            { "Threshold", "Threshold", "Processing" },
        });

        // Set connection creation callback for undo/redo
        connectionManager.SetConnectionCreatedCallback([this](const Widgets::NodeConnection &connection) {
            auto command = std::make_unique<Editor::Commands::CreateConnectionCommand>(
                connection.outputPin,
                connection.inputPin,
                [this](const Widgets::PinId &outputPin, const Widgets::PinId &inputPin) {
                    // Pass false to avoid triggering callback again
                    connectionManager.CreateConnection(outputPin, inputPin, this->nodeEditor, false);
                },
                [this](const Widgets::NodeConnection &conn) { connectionManager.RemoveConnection(conn); });

            commandHistory.ExecuteCommand(std::move(command));
        });
    }

    void NodeEditorLayer::OnEvent(Kappa::Event &event)
    {
        if (dynamic_cast<Events::SaveGraphEvent *>(&event))
        {
            HandleSaveGraph();
        }
        else if (dynamic_cast<Events::LoadGraphEvent *>(&event))
        {
            HandleLoadGraph();
        }
        else if (auto *loadFileEvent = dynamic_cast<Events::LoadGraphFromFileEvent *>(&event))
        {
            HandleLoadGraphFromFile(loadFileEvent->GetFilePath());
        }
        else if (dynamic_cast<Events::NewGraphEvent *>(&event))
        {
            HandleNewGraph();
        }
    }

    void NodeEditorLayer::OnUpdate(float deltaTime)
    {
    }

    void NodeEditorLayer::OnRender()
    {
        ImGui::Begin("Nodes::Node Editor");

        auto *drawList = ImGui::GetWindowDrawList();
        const auto canvasPos = ImGui::GetCursorScreenPos();
        const auto canvasSize = ImGui::GetContentRegionAvail();

        canvas.BeginCanvas(drawList, canvasPos, canvasSize);

        const auto &io = ImGui::GetIO();
        canvas.HandleImGuiInput(io, ImGui::IsWindowHovered());

        if (nodeEditor.GetNodeIds().empty())
        {
            const auto nodeId = AllocateNodeId();
            if (nodeId == 0)
            {
                return;
            }
            auto starterNode = std::make_unique<Vision::IO::ImageInputNode>(nodeId);
            nodeEditor.AddNode(std::move(starterNode));
            nodePositions[nodeId] = { 100.0f, 100.0f };

            selectionManager.ClearSelection();
        }

        HandleMouseInteractions();
        DetectHoveredPin();
        connectionManager.HandleConnectionInteractions(nodeEditor, nodePositions, canvas);

        connectionManager.RenderConnections(nodeEditor, nodePositions, canvas, hoveredConnection);
        RenderNodes();
        RenderBoxSelection();

        RenderContextMenu();

        canvas.EndCanvas();

        ImGui::End();

        // Render file dialogs
        RenderSaveDialog();
        RenderLoadDialog();
    }

    void NodeEditorLayer::RenderNodes()
    {
        for (const auto nodeId : nodeEditor.GetNodeIds())
        {
            auto *node = nodeEditor.GetNode(nodeId);
            if (node && nodePositions.find(nodeId) != nodePositions.end())
            {
                RenderNode(node, nodePositions[nodeId]);
            }
        }
    }

    void NodeEditorLayer::RenderNode(Nodes::Node *node, const Widgets::NodePosition &nodePos)
    {
        auto getPinInteractionState = [this](Nodes::NodeId nodeId,
                                          const std::string &pinName) -> Rendering::PinInteractionState {
            return this->GetPinInteractionState(nodeId, pinName);
        };

        // Determine if this node is selected
        const bool isSelected = selectionManager.IsNodeSelected(node->GetId());
        const Nodes::NodeId displaySelectedId = isSelected ? node->GetId() : Constants::Special::kInvalidNodeId;

        nodeRenderer.RenderNode(node, nodePos, displaySelectedId, getPinInteractionState);
    }

    bool NodeEditorLayer::IsMouseOverNode(const ImVec2 &mousePos,
        const Widgets::NodePosition &nodePos,
        const ImVec2 &nodeSize) const
    {
        const auto worldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        return mousePos.x >= worldPos.x && mousePos.x <= worldPos.x + nodeSize.x && mousePos.y >= worldPos.y
               && mousePos.y <= worldPos.y + nodeSize.y;
    }

    void NodeEditorLayer::HandleMouseInteractions()
    {
        // Create callbacks for InputHandler
        auto findNode = [this](const ImVec2 &pos) { return FindNodeAtPosition(pos); };

        auto findConnection = [this](const ImVec2 &pos) {
            return connectionManager.FindConnectionAtPosition(pos, this->nodeEditor, nodePositions, canvas);
        };

        auto updateBoxSelection = [this]() { UpdateBoxSelection(); };

        // Process input and get actions
        const auto actions =
            inputHandler.ProcessInput(nodePositions, hoveredPin, findNode, findConnection, updateBoxSelection);

        // Handle actions
        for (const auto &action : actions)
        {
            switch (action.type)
            {
            case Canvas::InputActionType::DeleteNodes:
                for (const auto nodeId : action.nodeIds)
                {
                    auto *node = nodeEditor.GetNode(nodeId);
                    if (!node)
                        continue;

                    auto command = std::make_unique<Editor::Commands::DeleteNodeCommand>(
                        nodeId,
                        [this](Nodes::NodeId id) -> Nodes::Node * { return nodeEditor.GetNode(id); },
                        [this](Nodes::NodeId id) {
                            nodeEditor.RemoveNode(id);
                            nodePositions.erase(id);
                            if (selectionManager.IsNodeSelected(id))
                            {
                                selectionManager.RemoveFromSelection(id);
                            }
                        },
                        [this](std::unique_ptr<Nodes::Node> node) { nodeEditor.AddNode(std::move(node)); },
                        [this](Nodes::NodeId id) -> Widgets::NodePosition { return nodePositions[id]; },
                        [this](Nodes::NodeId id, const Widgets::NodePosition &pos) { nodePositions[id] = pos; },
                        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
                            return Vision::NodeFactory::CreateNode(NodeTypeToFactoryKey(type), id, name);
                        });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                selectionManager.ClearSelection();
                break;

            case Canvas::InputActionType::DeleteConnection:
                if (action.connection.has_value())
                {
                    auto command = std::make_unique<Editor::Commands::DeleteConnectionCommand>(
                        action.connection.value(),
                        [this](const Widgets::PinId &outputPin, const Widgets::PinId &inputPin) {
                            connectionManager.CreateConnection(outputPin, inputPin, nodeEditor, false);
                        },
                        [this](const Widgets::NodeConnection &conn) { connectionManager.RemoveConnection(conn); });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                break;

            case Canvas::InputActionType::UpdateNodePositions:
                for (const auto &[nodeId, pos] : action.nodePositions)
                {
                    nodePositions[nodeId] = pos;
                }
                break;

            case Canvas::InputActionType::UpdateHoveredConnection:
                hoveredConnection = action.hoveredConnection;
                break;

            case Canvas::InputActionType::OpenContextMenu:
                // Context menu is already opened by InputHandler
                break;

            case Canvas::InputActionType::CopyNodes: {
                // Get node types and names from Nodes::NodeEditor
                std::unordered_map<Nodes::NodeId, std::string> nodeTypes;
                std::unordered_map<Nodes::NodeId, std::string> nodeNames;
                for (const auto nodeId : selectionManager.GetSelectedNodes())
                {
                    if (auto *node = nodeEditor.GetNode(nodeId))
                    {
                        // Convert node class type to factory key
                        nodeTypes[nodeId] = NodeTypeToFactoryKey(node->GetType());
                        nodeNames[nodeId] = node->GetName();
                    }
                }

                // Copy to clipboard
                clipboardManager.Copy(selectionManager.GetSelectedNodes(),
                    nodeTypes,
                    nodeNames,
                    nodePositions,
                    connectionManager.GetConnections());
                break;
            }

            case Canvas::InputActionType::CutNodes: {
                // Get node types and names from Nodes::NodeEditor
                std::unordered_map<Nodes::NodeId, std::string> nodeTypes;
                std::unordered_map<Nodes::NodeId, std::string> nodeNames;
                for (const auto nodeId : selectionManager.GetSelectedNodes())
                {
                    if (auto *node = nodeEditor.GetNode(nodeId))
                    {
                        // Convert node class type to factory key
                        nodeTypes[nodeId] = NodeTypeToFactoryKey(node->GetType());
                        nodeNames[nodeId] = node->GetName();
                    }
                }

                // Cut to clipboard
                clipboardManager.Cut(selectionManager.GetSelectedNodes(),
                    nodeTypes,
                    nodeNames,
                    nodePositions,
                    connectionManager.GetConnections());

                // Delete nodes immediately (they'll be restored on paste)
                std::vector<Nodes::NodeId> nodesToDelete(
                    selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
                for (const auto nodeId : nodesToDelete)
                {
                    DeleteNode(nodeId);
                }
                selectionManager.ClearSelection();
                break;
            }

            case Canvas::InputActionType::PasteNodes: {
                if (!clipboardManager.HasData())
                {
                    break;
                }

                // Convert mouse position to world coordinates
                const auto pasteWorldPos = canvas.ScreenToWorld(action.pastePosition);

                // Get copied nodes once and store reference
                const auto &copiedNodes = clipboardManager.GetCopiedNodes();
                if (copiedNodes.empty())
                {
                    break;
                }

                // Calculate center of copied nodes
                float centerX = 0.0f, centerY = 0.0f;
                for (const auto &copiedNode : copiedNodes)
                {
                    centerX += copiedNode.position.x;
                    centerY += copiedNode.position.y;
                }
                centerX /= copiedNodes.size();
                centerY /= copiedNodes.size();

                // Map old IDs to new IDs for connection remapping
                std::unordered_map<Nodes::NodeId, Nodes::NodeId> idMapping;

                // Create new nodes
                for (const auto &copiedNode : copiedNodes)
                {
                    const auto newNodeId = AllocateNodeId();
                    if (newNodeId == 0)
                    {
                        break; // Stop if we can't allocate more IDs
                    }
                    idMapping[copiedNode.originalId] = newNodeId;

                    // Create node using factory
                    auto newNode = Vision::NodeFactory::CreateNode(copiedNode.type, newNodeId, copiedNode.name);
                    if (newNode)
                    {
                        // Calculate offset from center and apply to paste position
                        const auto offsetX = copiedNode.position.x - centerX;
                        const auto offsetY = copiedNode.position.y - centerY;
                        const auto newX = pasteWorldPos.x + offsetX;
                        const auto newY = pasteWorldPos.y + offsetY;
                        nodePositions[newNodeId] = { newX, newY };

                        nodeEditor.AddNode(std::move(newNode));
                    }
                }

                // Recreate connections using new IDs
                for (const auto &copiedConnection : clipboardManager.GetCopiedConnections())
                {
                    const auto newFromId = idMapping[copiedConnection.fromNodeId];
                    const auto newToId = idMapping[copiedConnection.toNodeId];

                    Widgets::PinId outputPin{ newFromId, copiedConnection.fromSlot };
                    Widgets::PinId inputPin{ newToId, copiedConnection.toSlot };

                    connectionManager.CreateConnection(outputPin, inputPin, nodeEditor);
                }

                // Select the newly pasted nodes
                selectionManager.ClearSelection();
                for (const auto &[oldId, newId] : idMapping)
                {
                    selectionManager.ToggleNodeSelection(newId);
                }

                // After first paste of cut operation, convert to copy
                if (clipboardManager.GetOperation() == Editor::State::ClipboardOperation::Cut)
                {
                    clipboardManager.CompleteCutOperation();
                }

                break;
            }

            case Canvas::InputActionType::Undo:
                commandHistory.Undo();
                break;

            case Canvas::InputActionType::Redo:
                commandHistory.Redo();
                break;

            case Canvas::InputActionType::FinishNodeMove:
                if (!action.oldNodePositions.empty() && !action.nodePositions.empty())
                {
                    auto command = std::make_unique<Editor::Commands::MoveNodesCommand>(action.oldNodePositions,
                        action.nodePositions,
                        [this](Nodes::NodeId id, const Widgets::NodePosition &pos) { nodePositions[id] = pos; });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                break;

            case Canvas::InputActionType::None:
                break;
            }
        }
    }

    void NodeEditorLayer::DetectHoveredPin()
    {
        const auto &io = ImGui::GetIO();
        const auto mousePos = io.MousePos;

        if (!ImGui::IsWindowHovered())
        {
            hoveredPin = { Constants::Special::kInvalidNodeId, "" };
            return;
        }

        const auto hoveredNodeId = FindNodeAtPosition(mousePos);
        if (hoveredNodeId == Constants::Special::kInvalidNodeId)
        {
            hoveredPin = { Constants::Special::kInvalidNodeId, "" };
            return;
        }

        hoveredPin =
            connectionManager.FindPinAtPositionInNode(mousePos, hoveredNodeId, nodeEditor, nodePositions, canvas);
    }

    Rendering::PinInteractionState NodeEditorLayer::GetPinInteractionState(Nodes::NodeId nodeId,
        const std::string &pinName) const
    {
        Rendering::PinInteractionState state;
        state.isHovered = (hoveredPin.nodeId == nodeId && hoveredPin.pinName == pinName);
        state.isActive = (connectionManager.IsCreatingConnection() && connectionManager.GetStartPin().nodeId == nodeId
                          && connectionManager.GetStartPin().pinName == pinName);
        return state;
    }


    void NodeEditorLayer::RenderContextMenu()
    {
        const auto result = contextMenuRenderer.Render(selectionManager.HasSelection(), clipboardManager.HasData());

        switch (result.action)
        {
        case Widgets::ContextMenuResult::Action::DeleteNodes: {
            std::vector<Nodes::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                auto *node = nodeEditor.GetNode(nodeId);
                if (!node)
                    continue;

                auto command = std::make_unique<Editor::Commands::DeleteNodeCommand>(
                    nodeId,
                    [this](Nodes::NodeId id) -> Nodes::Node * { return nodeEditor.GetNode(id); },
                    [this](Nodes::NodeId id) {
                        nodeEditor.RemoveNode(id);
                        nodePositions.erase(id);
                        if (selectionManager.IsNodeSelected(id))
                        {
                            selectionManager.RemoveFromSelection(id);
                        }
                    },
                    [this](std::unique_ptr<Nodes::Node> node) { nodeEditor.AddNode(std::move(node)); },
                    [this](Nodes::NodeId id) -> Widgets::NodePosition { return nodePositions[id]; },
                    [this](Nodes::NodeId id, const Widgets::NodePosition &pos) { nodePositions[id] = pos; },
                    [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
                        return Vision::NodeFactory::CreateNode(NodeTypeToFactoryKey(type), id, name);
                    });

                commandHistory.ExecuteCommand(std::move(command));
            }
            selectionManager.ClearSelection();
            break;
        }
        case Widgets::ContextMenuResult::Action::CreateNode:
            CreateNodeAtPosition(result.nodeType, inputHandler.GetContextMenuPos());
            break;
        case Widgets::ContextMenuResult::Action::CopyNodes: {
            // Get node types and names from Nodes::NodeEditor
            std::unordered_map<Nodes::NodeId, std::string> nodeTypes;
            std::unordered_map<Nodes::NodeId, std::string> nodeNames;
            for (const auto nodeId : selectionManager.GetSelectedNodes())
            {
                if (auto *node = nodeEditor.GetNode(nodeId))
                {
                    // Convert node class type to factory key
                    nodeTypes[nodeId] = NodeTypeToFactoryKey(node->GetType());
                    nodeNames[nodeId] = node->GetName();
                }
            }

            // Copy to clipboard
            clipboardManager.Copy(selectionManager.GetSelectedNodes(),
                nodeTypes,
                nodeNames,
                nodePositions,
                connectionManager.GetConnections());
            break;
        }
        case Widgets::ContextMenuResult::Action::CutNodes: {
            // Get node types and names from Nodes::NodeEditor
            std::unordered_map<Nodes::NodeId, std::string> nodeTypes;
            std::unordered_map<Nodes::NodeId, std::string> nodeNames;
            for (const auto nodeId : selectionManager.GetSelectedNodes())
            {
                if (auto *node = nodeEditor.GetNode(nodeId))
                {
                    // Convert node class type to factory key
                    nodeTypes[nodeId] = NodeTypeToFactoryKey(node->GetType());
                    nodeNames[nodeId] = node->GetName();
                }
            }

            // Cut to clipboard
            clipboardManager.Cut(selectionManager.GetSelectedNodes(),
                nodeTypes,
                nodeNames,
                nodePositions,
                connectionManager.GetConnections());

            // Delete nodes immediately (they'll be restored on paste)
            std::vector<Nodes::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                DeleteNode(nodeId);
            }
            selectionManager.ClearSelection();
            break;
        }
        case Widgets::ContextMenuResult::Action::PasteNodes: {
            if (!clipboardManager.HasData())
            {
                break;
            }

            // Convert context menu position to world coordinates
            const auto pasteWorldPos = canvas.ScreenToWorld(inputHandler.GetContextMenuPos());

            // Get copied nodes once and store reference
            const auto &copiedNodes = clipboardManager.GetCopiedNodes();
            if (copiedNodes.empty())
            {
                break;
            }

            // Calculate center of copied nodes
            float centerX = 0.0f, centerY = 0.0f;
            for (const auto &copiedNode : copiedNodes)
            {
                centerX += copiedNode.position.x;
                centerY += copiedNode.position.y;
            }
            centerX /= copiedNodes.size();
            centerY /= copiedNodes.size();

            // Map old IDs to new IDs for connection remapping
            std::unordered_map<Nodes::NodeId, Nodes::NodeId> idMapping;

            // Create new nodes
            for (const auto &copiedNode : copiedNodes)
            {
                const auto newNodeId = AllocateNodeId();
                if (newNodeId == 0)
                {
                    break; // Stop if we can't allocate more IDs
                }
                idMapping[copiedNode.originalId] = newNodeId;

                // Create node using factory
                auto newNode = Vision::NodeFactory::CreateNode(copiedNode.type, newNodeId, copiedNode.name);
                if (newNode)
                {
                    // Calculate offset from center and apply to paste position
                    const auto offsetX = copiedNode.position.x - centerX;
                    const auto offsetY = copiedNode.position.y - centerY;
                    const auto newX = pasteWorldPos.x + offsetX;
                    const auto newY = pasteWorldPos.y + offsetY;
                    nodePositions[newNodeId] = { newX, newY };

                    nodeEditor.AddNode(std::move(newNode));
                }
            }

            // Recreate connections using new IDs
            for (const auto &copiedConnection : clipboardManager.GetCopiedConnections())
            {
                const auto newFromId = idMapping[copiedConnection.fromNodeId];
                const auto newToId = idMapping[copiedConnection.toNodeId];

                Widgets::PinId outputPin{ newFromId, copiedConnection.fromSlot };
                Widgets::PinId inputPin{ newToId, copiedConnection.toSlot };

                connectionManager.CreateConnection(outputPin, inputPin, nodeEditor);
            }

            // Select the newly pasted nodes
            selectionManager.ClearSelection();
            for (const auto &[oldId, newId] : idMapping)
            {
                selectionManager.ToggleNodeSelection(newId);
            }

            // After first paste of cut operation, convert to copy
            if (clipboardManager.GetOperation() == Editor::State::ClipboardOperation::Cut)
            {
                clipboardManager.CompleteCutOperation();
            }

            break;
        }
        case Widgets::ContextMenuResult::Action::None:
            break;
        }
    }

    void NodeEditorLayer::CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position)
    {
        const auto worldPos = canvas.ScreenToWorld(position);
        const auto worldX = worldPos.x - Constants::Node::Creation::kOffsetX;
        const auto worldY = worldPos.y - Constants::Node::Creation::kOffsetY;

        const auto nodeId = AllocateNodeId();
        if (nodeId == 0)
        {
            LOG_ERROR("NodeEditorLayer: Cannot create node - ID allocation failed");
            return;
        }

        // Map node types to display names
        static const std::unordered_map<std::string, std::string> displayNames = { { "ImageInput", "Image Input" },
            { "ImageOutput", "Image Output" },
            { "Grayscale", "Grayscale" },
            { "CannyEdge", "Canny Edge" },
            { "Threshold", "Threshold" },
            { "Preview", "Preview" } };

        // Get display name or use type as fallback
        const auto displayName = displayNames.contains(nodeType) ? displayNames.at(nodeType) : nodeType;

        // Create command for node creation
        auto command = std::make_unique<Editor::Commands::CreateNodeCommand>(
            [this, nodeType, nodeId, displayName]() {
                return Vision::NodeFactory::CreateNode(nodeType, nodeId, displayName);
            },
            [this](std::unique_ptr<Nodes::Node> node) { nodeEditor.AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { nodeEditor.RemoveNode(id); },
            [this](Nodes::NodeId id, const Widgets::NodePosition &pos) { nodePositions[id] = pos; },
            Widgets::NodePosition{ worldX, worldY },
            nodeType);

        commandHistory.ExecuteCommand(std::move(command));
        selectionManager.ClearSelection();
    }


    ImU32 NodeEditorLayer::GetDataTypeColor(Widgets::PinDataType dataType) const
    {
        switch (dataType)
        {
        case Widgets::PinDataType::Image:
            return Constants::Colors::Pin::kImage;
        case Widgets::PinDataType::String:
            return Constants::Colors::Pin::kString;
        case Widgets::PinDataType::Float:
            return Constants::Colors::Pin::kFloat;
        case Widgets::PinDataType::Int:
            return Constants::Colors::Pin::kInt;
        case Widgets::PinDataType::Bool:
            return Constants::Colors::Pin::kBool;
        case Widgets::PinDataType::Path:
            return Constants::Colors::Pin::kPath;
        default:
            return Constants::Colors::Pin::kDefault;
        }
    }


    Nodes::NodeId NodeEditorLayer::FindNodeAtPosition(const ImVec2 &mousePos) const
    {
        const auto nodeIds = nodeEditor.GetNodeIds();

        for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
        {
            const auto nodeId = *it;
            const auto *node = nodeEditor.GetNode(nodeId);

            if (!node || nodePositions.find(nodeId) == nodePositions.end())
                continue;

            const auto pins = connectionManager.GetNodePins(node->GetName());
            const auto dimensions = Rendering::NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);

            if (IsMouseOverNode(mousePos, nodePositions.at(nodeId), dimensions.size))
            {
                return nodeId;
            }
        }

        return Constants::Special::kInvalidNodeId;
    }


    void NodeEditorLayer::HandleSaveGraph()
    {
        if (currentFilePath.empty())
        {
            fileDialogManager.OpenSaveDialog();
        }
        else
        {
            std::unordered_map<Nodes::NodeId, std::pair<float, float>> positions;
            for (const auto &[id, pos] : nodePositions)
            {
                positions[id] = { pos.x, pos.y };
            }

            if (nodeEditor.SaveToFile(currentFilePath, positions))
            {
                LOG_INFO("Graph saved successfully to: {}", currentFilePath);

                Events::FileOpenedEvent fileEvent(currentFilePath);
                Kappa::Application::Get().GetEventBus().Publish(fileEvent);
            }
            else
            {
                LOG_ERROR("Failed to save graph");
            }
        }
    }

    void NodeEditorLayer::HandleLoadGraph()
    {
        fileDialogManager.OpenLoadDialog();
    }

    void NodeEditorLayer::HandleLoadGraphFromFile(const std::string &filePath)
    {
        std::unordered_map<Nodes::NodeId, std::pair<float, float>> positions;

        if (nodeEditor.LoadFromFile(filePath, positions))
        {
            nodePositions.clear();
            for (const auto &[id, pos] : positions)
            {
                nodePositions[id] = Widgets::NodePosition{ pos.first, pos.second };
            }

            currentFilePath = filePath;
            selectionManager.ClearSelection();

            Nodes::NodeId maxId = 0;
            for (const auto &id : nodeEditor.GetNodeIds())
            {
                if (id > maxId)
                    maxId = id;
            }
            nextNodeId = maxId + 1;

            LOG_INFO("Graph loaded successfully from: {}", currentFilePath);

            Events::FileOpenedEvent fileEvent(currentFilePath);
            Kappa::Application::Get().GetEventBus().Publish(fileEvent);
        }
        else
        {
            LOG_ERROR("Failed to load graph from: {}", filePath);
        }
    }

    void NodeEditorLayer::HandleNewGraph()
    {
        nodeEditor.Clear();
        nodePositions.clear();
        currentFilePath.clear();
        selectionManager.ClearSelection();
        nextNodeId = 1;
        LOG_INFO("Created new graph");
    }

    void NodeEditorLayer::RenderSaveDialog()
    {
        const auto result = fileDialogManager.RenderSaveDialog();

        if (result.action == Widgets::FileDialogResult::Action::Save)
        {
            currentFilePath = result.filepath;

            std::unordered_map<Nodes::NodeId, std::pair<float, float>> positions;
            for (const auto &[id, pos] : nodePositions)
            {
                positions[id] = { pos.x, pos.y };
            }

            if (nodeEditor.SaveToFile(currentFilePath, positions))
            {
                LOG_INFO("Graph saved successfully to: {}", currentFilePath);

                Events::FileOpenedEvent fileEvent(currentFilePath);
                Kappa::Application::Get().GetEventBus().Publish(fileEvent);
            }
            else
            {
                LOG_ERROR("Failed to save graph");
            }
        }
    }

    void NodeEditorLayer::RenderLoadDialog()
    {
        const auto result = fileDialogManager.RenderLoadDialog();

        if (result.action == Widgets::FileDialogResult::Action::Load)
        {
            std::unordered_map<Nodes::NodeId, std::pair<float, float>> positions;

            if (nodeEditor.LoadFromFile(result.filepath, positions))
            {
                nodePositions.clear();
                for (const auto &[id, pos] : positions)
                {
                    nodePositions[id] = Widgets::NodePosition{ pos.first, pos.second };
                }

                currentFilePath = result.filepath;
                selectionManager.ClearSelection();

                // Update nextNodeId to be higher than any loaded ID
                Nodes::NodeId maxId = 0;
                for (const auto &id : nodeEditor.GetNodeIds())
                {
                    if (id > maxId)
                        maxId = id;
                }
                nextNodeId = maxId + 1;

                LOG_INFO("Graph loaded successfully from: {}", currentFilePath);

                Events::FileOpenedEvent fileEvent(currentFilePath);
                Kappa::Application::Get().GetEventBus().Publish(fileEvent);
            }
            else
            {
                LOG_ERROR("Failed to load graph from: {}", result.filepath);
            }
        }
    }

    void NodeEditorLayer::DeleteNode(Nodes::NodeId nodeId)
    {
        if (nodeId == Constants::Special::kInvalidNodeId)
        {
            return;
        }

        // Remove from selection if it's selected
        if (selectionManager.IsNodeSelected(nodeId))
        {
            selectionManager.RemoveFromSelection(nodeId);
        }

        // Remove connections from UI layer
        connectionManager.RemoveConnectionsForNode(nodeId);

        // Remove node (also removes connections from core layer)
        nodeEditor.RemoveNode(nodeId);

        // Remove node position
        nodePositions.erase(nodeId);
    }

    void NodeEditorLayer::RenderBoxSelection()
    {
        if (!selectionManager.IsBoxSelecting())
        {
            return;
        }

        auto *drawList = ImGui::GetWindowDrawList();

        // Get box selection bounds
        const auto [start, end] = selectionManager.GetBoxSelectionBounds();
        const ImVec2 minPos(std::min(start.x, end.x), std::min(start.y, end.y));
        const ImVec2 maxPos(std::max(start.x, end.x), std::max(start.y, end.y));

        // Draw selection box
        const ImU32 boxColor = IM_COL32(100, 150, 255, 100);
        const ImU32 borderColor = IM_COL32(100, 150, 255, 200);

        drawList->AddRectFilled(minPos, maxPos, boxColor);
        drawList->AddRect(minPos, maxPos, borderColor, 0.0f, 0, 2.0f);
    }

    void NodeEditorLayer::UpdateBoxSelection()
    {
        // Get box selection bounds
        const auto [start, end] = selectionManager.GetBoxSelectionBounds();
        const ImVec2 minPos(std::min(start.x, end.x), std::min(start.y, end.y));
        const ImVec2 maxPos(std::max(start.x, end.x), std::max(start.y, end.y));

        // Check each node
        for (const auto &[nodeId, nodePos] : nodePositions)
        {
            const auto screenPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
            const auto nodeSize = ImVec2(
                Constants::Node::kWidth * canvas.GetZoomLevel(), Constants::Node::kMinHeight * canvas.GetZoomLevel());

            // Check if node overlaps with selection box
            const bool overlapsX = screenPos.x < maxPos.x && (screenPos.x + nodeSize.x) > minPos.x;
            const bool overlapsY = screenPos.y < maxPos.y && (screenPos.y + nodeSize.y) > minPos.y;

            if (overlapsX && overlapsY)
            {
                selectionManager.AddToSelection(nodeId);
            }
        }
    }

    bool NodeEditorLayer::IsNodeSelected(Nodes::NodeId nodeId) const
    {
        return selectionManager.IsNodeSelected(nodeId);
    }

    std::string NodeEditorLayer::NodeTypeToFactoryKey(const std::string &nodeType) const
    {
        // Mapping from node class type to factory registration key
        static const std::unordered_map<std::string, std::string> typeToFactoryKey = { { "ImageInputNode",
                                                                                           "ImageInput" },
            { "ImageOutputNode", "ImageOutput" },
            { "PreviewNode", "Preview" },
            { "GrayscaleNode", "Grayscale" },
            { "CannyEdgeNode", "CannyEdge" },
            { "ThresholdNode", "Threshold" } };

        if (typeToFactoryKey.contains(nodeType))
        {
            return typeToFactoryKey.at(nodeType);
        }

        // Fallback: return the original type
        LOG_WARN("NodeEditorLayer::NodeTypeToFactoryKey - Unknown node type: {}", nodeType);
        return nodeType;
    }

    Nodes::NodeId NodeEditorLayer::AllocateNodeId()
    {
        constexpr Nodes::NodeId kMaxNodeId = std::numeric_limits<Nodes::NodeId>::max() - 1;
        if (nextNodeId >= kMaxNodeId)
        {
            LOG_ERROR("NodeEditorLayer: Node ID overflow - cannot create more nodes");
            return 0; // Invalid node ID
        }
        return nextNodeId++;
    }
} // namespace VisionCraft::UI::Layers
