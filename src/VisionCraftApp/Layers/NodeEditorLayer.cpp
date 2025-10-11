#include "NodeEditorLayer.h"
#include "Commands/ConnectionCommands.h"
#include "Commands/NodeCommands.h"
#include "Editor/NodeEditorConstants.h"
#include "Events/FileOpenedEvent.h"
#include "Events/LoadGraphEvent.h"
#include "Events/NewGraphEvent.h"
#include "Events/SaveGraphEvent.h"
#include "Logger.h"
#include "Rendering/NodeRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>

#include <imgui.h>

#include "Application.h"
#include "Main/VisionCraftApplication.h"

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/PreviewNode.h"
#include "Nodes/ThresholdNode.h"


namespace VisionCraft
{
    NodeEditorLayer::NodeEditorLayer()
        : nodeRenderer(canvas, connectionManager), inputHandler(selectionManager, contextMenuRenderer, canvas)
    {
        // Register all available node types with the factory
        nodeFactory.Register("ImageInput", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::ImageInputNode>(id, std::string(name));
        });

        nodeFactory.Register("ImageOutput", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::ImageOutputNode>(id, std::string(name));
        });

        nodeFactory.Register("Preview", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::PreviewNode>(id, std::string(name));
        });

        nodeFactory.Register("Grayscale", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::GrayscaleNode>(id, std::string(name));
        });

        nodeFactory.Register("CannyEdge", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::CannyEdgeNode>(id, std::string(name));
        });

        nodeFactory.Register("Threshold", [](Engine::NodeId id, std::string_view name) {
            return std::make_unique<Engine::ThresholdNode>(id, std::string(name));
        });

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
        connectionManager.SetConnectionCreatedCallback([this](const NodeConnection &connection) {
            auto command = std::make_unique<CreateConnectionCommand>(
                connection.outputPin,
                connection.inputPin,
                [this](const PinId &outputPin, const PinId &inputPin) {
                    // Pass false to avoid triggering callback again
                    connectionManager.CreateConnection(outputPin, inputPin, GetNodeEditor(), false);
                },
                [this](const NodeConnection &conn) { connectionManager.RemoveConnection(conn); });

            commandHistory.ExecuteCommand(std::move(command));
        });
    }

    void NodeEditorLayer::OnEvent(Kappa::Event &event)
    {
        if (dynamic_cast<SaveGraphEvent *>(&event))
        {
            HandleSaveGraph();
        }
        else if (dynamic_cast<LoadGraphEvent *>(&event))
        {
            HandleLoadGraph();
        }
        else if (auto *loadFileEvent = dynamic_cast<LoadGraphFromFileEvent *>(&event))
        {
            HandleLoadGraphFromFile(loadFileEvent->GetFilePath());
        }
        else if (dynamic_cast<NewGraphEvent *>(&event))
        {
            HandleNewGraph();
        }
    }

    void NodeEditorLayer::OnUpdate(float deltaTime)
    {
    }

    void NodeEditorLayer::OnRender()
    {
        ImGui::Begin("Node Editor");

        auto *drawList = ImGui::GetWindowDrawList();
        const auto canvasPos = ImGui::GetCursorScreenPos();
        const auto canvasSize = ImGui::GetContentRegionAvail();

        canvas.BeginCanvas(drawList, canvasPos, canvasSize);

        const auto &io = ImGui::GetIO();
        canvas.HandleImGuiInput(io, ImGui::IsWindowHovered());

        auto &nodeEditor = GetNodeEditor();

        if (nodeEditor.GetNodeIds().empty())
        {
            auto starterNode = std::make_unique<Engine::ImageInputNode>(nextNodeId++);
            const auto nodeId = starterNode->GetId();
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
        auto &nodeEditor = GetNodeEditor();

        for (const auto nodeId : nodeEditor.GetNodeIds())
        {
            auto *node = nodeEditor.GetNode(nodeId);
            if (node && nodePositions.find(nodeId) != nodePositions.end())
            {
                RenderNode(node, nodePositions[nodeId]);
            }
        }
    }

    void NodeEditorLayer::RenderNode(Engine::Node *node, const NodePosition &nodePos)
    {
        auto getPinInteractionState = [this](Engine::NodeId nodeId, const std::string &pinName) -> PinInteractionState {
            return this->GetPinInteractionState(nodeId, pinName);
        };

        // Determine if this node is selected
        const bool isSelected = selectionManager.IsNodeSelected(node->GetId());
        const Engine::NodeId displaySelectedId = isSelected ? node->GetId() : Constants::Special::kInvalidNodeId;

        nodeRenderer.RenderNode(node, nodePos, displaySelectedId, getPinInteractionState);
    }

    bool NodeEditorLayer::IsMouseOverNode(const ImVec2 &mousePos,
        const NodePosition &nodePos,
        const ImVec2 &nodeSize) const
    {
        const auto worldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        return mousePos.x >= worldPos.x && mousePos.x <= worldPos.x + nodeSize.x && mousePos.y >= worldPos.y
               && mousePos.y <= worldPos.y + nodeSize.y;
    }

    void NodeEditorLayer::HandleMouseInteractions()
    {
        auto &nodeEditor = GetNodeEditor();

        // Create callbacks for InputHandler
        auto findNode = [this](const ImVec2 &pos) { return FindNodeAtPosition(pos); };

        auto findConnection = [this, &nodeEditor](const ImVec2 &pos) {
            return connectionManager.FindConnectionAtPosition(pos, nodeEditor, nodePositions, canvas);
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
            case InputActionType::DeleteNodes:
                for (const auto nodeId : action.nodeIds)
                {
                    auto *node = nodeEditor.GetNode(nodeId);
                    if (!node)
                        continue;

                    auto command = std::make_unique<DeleteNodeCommand>(
                        nodeId,
                        [this](Engine::NodeId id) -> Engine::Node * { return GetNodeEditor().GetNode(id); },
                        [this](Engine::NodeId id) {
                            GetNodeEditor().RemoveNode(id);
                            nodePositions.erase(id);
                            if (selectionManager.IsNodeSelected(id))
                            {
                                selectionManager.RemoveFromSelection(id);
                            }
                        },
                        [this](std::unique_ptr<Engine::Node> node) { GetNodeEditor().AddNode(std::move(node)); },
                        [this](Engine::NodeId id) -> NodePosition { return nodePositions[id]; },
                        [this](Engine::NodeId id, const NodePosition &pos) { nodePositions[id] = pos; },
                        [this](const std::string &type, Engine::NodeId id, const std::string &name) {
                            return nodeFactory.Create(NodeTypeToFactoryKey(type), id, name);
                        });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                selectionManager.ClearSelection();
                break;

            case InputActionType::DeleteConnection:
                if (action.connection.has_value())
                {
                    auto command = std::make_unique<DeleteConnectionCommand>(
                        action.connection.value(),
                        [this](const PinId &outputPin, const PinId &inputPin) {
                            connectionManager.CreateConnection(outputPin, inputPin, GetNodeEditor(), false);
                        },
                        [this](const NodeConnection &conn) { connectionManager.RemoveConnection(conn); });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                break;

            case InputActionType::UpdateNodePositions:
                for (const auto &[nodeId, pos] : action.nodePositions)
                {
                    nodePositions[nodeId] = pos;
                }
                break;

            case InputActionType::UpdateHoveredConnection:
                hoveredConnection = action.hoveredConnection;
                break;

            case InputActionType::OpenContextMenu:
                // Context menu is already opened by InputHandler
                break;

            case InputActionType::CopyNodes: {
                // Get node types and names from NodeEditor
                std::unordered_map<Engine::NodeId, std::string> nodeTypes;
                std::unordered_map<Engine::NodeId, std::string> nodeNames;
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

            case InputActionType::CutNodes: {
                // Get node types and names from NodeEditor
                std::unordered_map<Engine::NodeId, std::string> nodeTypes;
                std::unordered_map<Engine::NodeId, std::string> nodeNames;
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
                std::vector<Engine::NodeId> nodesToDelete(
                    selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
                for (const auto nodeId : nodesToDelete)
                {
                    DeleteNode(nodeId);
                }
                selectionManager.ClearSelection();
                break;
            }

            case InputActionType::PasteNodes: {
                if (!clipboardManager.HasData())
                {
                    break;
                }

                // Convert mouse position to world coordinates
                const auto pasteWorldPos = canvas.ScreenToWorld(action.pastePosition);

                // Calculate center of copied nodes
                float centerX = 0.0f, centerY = 0.0f;
                for (const auto &copiedNode : clipboardManager.GetCopiedNodes())
                {
                    centerX += copiedNode.position.x;
                    centerY += copiedNode.position.y;
                }
                if (!clipboardManager.GetCopiedNodes().empty())
                {
                    centerX /= clipboardManager.GetCopiedNodes().size();
                    centerY /= clipboardManager.GetCopiedNodes().size();
                }

                // Map old IDs to new IDs for connection remapping
                std::unordered_map<Engine::NodeId, Engine::NodeId> idMapping;

                // Create new nodes
                for (const auto &copiedNode : clipboardManager.GetCopiedNodes())
                {
                    const auto newNodeId = nextNodeId++;
                    idMapping[copiedNode.originalId] = newNodeId;

                    // Create node using factory
                    auto newNode = nodeFactory.Create(copiedNode.type, newNodeId, copiedNode.name);
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

                    PinId outputPin{ newFromId, copiedConnection.fromSlot };
                    PinId inputPin{ newToId, copiedConnection.toSlot };

                    connectionManager.CreateConnection(outputPin, inputPin, nodeEditor);
                }

                // Select the newly pasted nodes
                selectionManager.ClearSelection();
                for (const auto &[oldId, newId] : idMapping)
                {
                    selectionManager.ToggleNodeSelection(newId);
                }

                // After first paste of cut operation, convert to copy
                if (clipboardManager.GetOperation() == ClipboardOperation::Cut)
                {
                    clipboardManager.CompleteCutOperation();
                }

                break;
            }

            case InputActionType::Undo:
                commandHistory.Undo();
                break;

            case InputActionType::Redo:
                commandHistory.Redo();
                break;

            case InputActionType::FinishNodeMove:
                if (!action.oldNodePositions.empty() && !action.nodePositions.empty())
                {
                    auto command = std::make_unique<MoveNodesCommand>(action.oldNodePositions,
                        action.nodePositions,
                        [this](Engine::NodeId id, const NodePosition &pos) { nodePositions[id] = pos; });

                    commandHistory.ExecuteCommand(std::move(command));
                }
                break;

            case InputActionType::None:
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

        auto &nodeEditor = GetNodeEditor();

        hoveredPin =
            connectionManager.FindPinAtPositionInNode(mousePos, hoveredNodeId, nodeEditor, nodePositions, canvas);
    }

    PinInteractionState NodeEditorLayer::GetPinInteractionState(Engine::NodeId nodeId, const std::string &pinName) const
    {
        PinInteractionState state;
        state.isHovered = (hoveredPin.nodeId == nodeId && hoveredPin.pinName == pinName);
        state.isActive = (connectionManager.IsCreatingConnection() && connectionManager.GetStartPin().nodeId == nodeId
                          && connectionManager.GetStartPin().pinName == pinName);
        return state;
    }


    void NodeEditorLayer::RenderContextMenu()
    {
        const auto result = contextMenuRenderer.Render(selectionManager.HasSelection(), clipboardManager.HasData());

        auto &nodeEditor = GetNodeEditor();

        switch (result.action)
        {
        case ContextMenuResult::Action::DeleteNodes: {
            std::vector<Engine::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                auto *node = nodeEditor.GetNode(nodeId);
                if (!node)
                    continue;

                auto command = std::make_unique<DeleteNodeCommand>(
                    nodeId,
                    [this](Engine::NodeId id) -> Engine::Node * { return GetNodeEditor().GetNode(id); },
                    [this](Engine::NodeId id) {
                        GetNodeEditor().RemoveNode(id);
                        nodePositions.erase(id);
                        if (selectionManager.IsNodeSelected(id))
                        {
                            selectionManager.RemoveFromSelection(id);
                        }
                    },
                    [this](std::unique_ptr<Engine::Node> node) { GetNodeEditor().AddNode(std::move(node)); },
                    [this](Engine::NodeId id) -> NodePosition { return nodePositions[id]; },
                    [this](Engine::NodeId id, const NodePosition &pos) { nodePositions[id] = pos; },
                    [this](const std::string &type, Engine::NodeId id, const std::string &name) {
                        return nodeFactory.Create(NodeTypeToFactoryKey(type), id, name);
                    });

                commandHistory.ExecuteCommand(std::move(command));
            }
            selectionManager.ClearSelection();
            break;
        }
        case ContextMenuResult::Action::CreateNode:
            CreateNodeAtPosition(result.nodeType, inputHandler.GetContextMenuPos());
            break;
        case ContextMenuResult::Action::CopyNodes: {
            // Get node types and names from NodeEditor
            std::unordered_map<Engine::NodeId, std::string> nodeTypes;
            std::unordered_map<Engine::NodeId, std::string> nodeNames;
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
        case ContextMenuResult::Action::CutNodes: {
            // Get node types and names from NodeEditor
            std::unordered_map<Engine::NodeId, std::string> nodeTypes;
            std::unordered_map<Engine::NodeId, std::string> nodeNames;
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
            std::vector<Engine::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                DeleteNode(nodeId);
            }
            selectionManager.ClearSelection();
            break;
        }
        case ContextMenuResult::Action::PasteNodes: {
            if (!clipboardManager.HasData())
            {
                break;
            }

            // Convert context menu position to world coordinates
            const auto pasteWorldPos = canvas.ScreenToWorld(inputHandler.GetContextMenuPos());

            // Calculate center of copied nodes
            float centerX = 0.0f, centerY = 0.0f;
            for (const auto &copiedNode : clipboardManager.GetCopiedNodes())
            {
                centerX += copiedNode.position.x;
                centerY += copiedNode.position.y;
            }
            if (!clipboardManager.GetCopiedNodes().empty())
            {
                centerX /= clipboardManager.GetCopiedNodes().size();
                centerY /= clipboardManager.GetCopiedNodes().size();
            }

            // Map old IDs to new IDs for connection remapping
            std::unordered_map<Engine::NodeId, Engine::NodeId> idMapping;

            // Create new nodes
            for (const auto &copiedNode : clipboardManager.GetCopiedNodes())
            {
                const auto newNodeId = nextNodeId++;
                idMapping[copiedNode.originalId] = newNodeId;

                // Create node using factory
                auto newNode = nodeFactory.Create(copiedNode.type, newNodeId, copiedNode.name);
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

                PinId outputPin{ newFromId, copiedConnection.fromSlot };
                PinId inputPin{ newToId, copiedConnection.toSlot };

                connectionManager.CreateConnection(outputPin, inputPin, nodeEditor);
            }

            // Select the newly pasted nodes
            selectionManager.ClearSelection();
            for (const auto &[oldId, newId] : idMapping)
            {
                selectionManager.ToggleNodeSelection(newId);
            }

            // After first paste of cut operation, convert to copy
            if (clipboardManager.GetOperation() == ClipboardOperation::Cut)
            {
                clipboardManager.CompleteCutOperation();
            }

            break;
        }
        case ContextMenuResult::Action::None:
            break;
        }
    }

    void NodeEditorLayer::CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position)
    {
        const auto worldPos = canvas.ScreenToWorld(position);
        const auto worldX = worldPos.x - Constants::Node::Creation::kOffsetX;
        const auto worldY = worldPos.y - Constants::Node::Creation::kOffsetY;

        const auto nodeId = nextNodeId++;

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
        auto command = std::make_unique<CreateNodeCommand>(
            [this, nodeType, nodeId, displayName]() { return nodeFactory.Create(nodeType, nodeId, displayName); },
            [this](std::unique_ptr<Engine::Node> node) { GetNodeEditor().AddNode(std::move(node)); },
            [this](Engine::NodeId id) { GetNodeEditor().RemoveNode(id); },
            [this](Engine::NodeId id, const NodePosition &pos) { nodePositions[id] = pos; },
            NodePosition{ worldX, worldY },
            nodeType);

        commandHistory.ExecuteCommand(std::move(command));
        selectionManager.ClearSelection();
    }


    ImU32 NodeEditorLayer::GetDataTypeColor(PinDataType dataType) const
    {
        switch (dataType)
        {
        case PinDataType::Image:
            return Constants::Colors::Pin::kImage;
        case PinDataType::String:
            return Constants::Colors::Pin::kString;
        case PinDataType::Float:
            return Constants::Colors::Pin::kFloat;
        case PinDataType::Int:
            return Constants::Colors::Pin::kInt;
        case PinDataType::Bool:
            return Constants::Colors::Pin::kBool;
        case PinDataType::Path:
            return Constants::Colors::Pin::kPath;
        default:
            return Constants::Colors::Pin::kDefault;
        }
    }


    Engine::NodeId NodeEditorLayer::FindNodeAtPosition(const ImVec2 &mousePos) const
    {
        const auto &nodeEditor = GetNodeEditor();
        const auto nodeIds = nodeEditor.GetNodeIds();

        for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
        {
            const auto nodeId = *it;
            const auto *node = nodeEditor.GetNode(nodeId);

            if (!node || nodePositions.find(nodeId) == nodePositions.end())
                continue;

            const auto pins = connectionManager.GetNodePins(node->GetName());
            const auto dimensions = NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);

            if (IsMouseOverNode(mousePos, nodePositions.at(nodeId), dimensions.size))
            {
                return nodeId;
            }
        }

        return Constants::Special::kInvalidNodeId;
    }

    Engine::NodeEditor &NodeEditorLayer::GetNodeEditor()
    {
        return static_cast<VisionCraftApplication &>(Kappa::Application::Get()).GetNodeEditor();
    }

    const Engine::NodeEditor &NodeEditorLayer::GetNodeEditor() const
    {
        return static_cast<const VisionCraftApplication &>(Kappa::Application::Get()).GetNodeEditor();
    }

    void NodeEditorLayer::HandleSaveGraph()
    {
        if (currentFilePath.empty())
        {
            fileDialogManager.OpenSaveDialog();
        }
        else
        {
            std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;
            for (const auto &[id, pos] : nodePositions)
            {
                positions[id] = { pos.x, pos.y };
            }

            if (GetNodeEditor().SaveToFile(currentFilePath, positions))
            {
                LOG_INFO("Graph saved successfully to: {}", currentFilePath);

                FileOpenedEvent fileEvent(currentFilePath);
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
        std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;

        if (GetNodeEditor().LoadFromFile(filePath, positions))
        {
            nodePositions.clear();
            for (const auto &[id, pos] : positions)
            {
                nodePositions[id] = NodePosition{ pos.first, pos.second };
            }

            currentFilePath = filePath;
            selectionManager.ClearSelection();

            Engine::NodeId maxId = 0;
            for (const auto &id : GetNodeEditor().GetNodeIds())
            {
                if (id > maxId)
                    maxId = id;
            }
            nextNodeId = maxId + 1;

            LOG_INFO("Graph loaded successfully from: {}", currentFilePath);

            FileOpenedEvent fileEvent(currentFilePath);
            Kappa::Application::Get().GetEventBus().Publish(fileEvent);
        }
        else
        {
            LOG_ERROR("Failed to load graph from: {}", filePath);
        }
    }

    void NodeEditorLayer::HandleNewGraph()
    {
        GetNodeEditor().Clear();
        nodePositions.clear();
        currentFilePath.clear();
        selectionManager.ClearSelection();
        nextNodeId = 1;
        LOG_INFO("Created new graph");
    }

    void NodeEditorLayer::RenderSaveDialog()
    {
        const auto result = fileDialogManager.RenderSaveDialog();

        if (result.action == FileDialogResult::Action::Save)
        {
            currentFilePath = result.filepath;

            std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;
            for (const auto &[id, pos] : nodePositions)
            {
                positions[id] = { pos.x, pos.y };
            }

            if (GetNodeEditor().SaveToFile(currentFilePath, positions))
            {
                LOG_INFO("Graph saved successfully to: {}", currentFilePath);

                FileOpenedEvent fileEvent(currentFilePath);
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

        if (result.action == FileDialogResult::Action::Load)
        {
            std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;

            if (GetNodeEditor().LoadFromFile(result.filepath, positions))
            {
                nodePositions.clear();
                for (const auto &[id, pos] : positions)
                {
                    nodePositions[id] = NodePosition{ pos.first, pos.second };
                }

                currentFilePath = result.filepath;
                selectionManager.ClearSelection();

                // Update nextNodeId to be higher than any loaded ID
                Engine::NodeId maxId = 0;
                for (const auto &id : GetNodeEditor().GetNodeIds())
                {
                    if (id > maxId)
                        maxId = id;
                }
                nextNodeId = maxId + 1;

                LOG_INFO("Graph loaded successfully from: {}", currentFilePath);

                FileOpenedEvent fileEvent(currentFilePath);
                Kappa::Application::Get().GetEventBus().Publish(fileEvent);
            }
            else
            {
                LOG_ERROR("Failed to load graph from: {}", result.filepath);
            }
        }
    }

    void NodeEditorLayer::DeleteNode(Engine::NodeId nodeId)
    {
        if (nodeId == Constants::Special::kInvalidNodeId)
        {
            return;
        }

        auto &nodeEditor = GetNodeEditor();

        // Remove from selection if it's selected
        if (selectionManager.IsNodeSelected(nodeId))
        {
            selectionManager.RemoveFromSelection(nodeId);
        }

        // Remove node (also removes connections)
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

    bool NodeEditorLayer::IsNodeSelected(Engine::NodeId nodeId) const
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
} // namespace VisionCraft
