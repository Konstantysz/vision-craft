#include "NodeEditorLayer.h"
#include "Events/LoadGraphEvent.h"
#include "Events/NewGraphEvent.h"
#include "Events/SaveGraphEvent.h"
#include "Logger.h"
#include "NodeEditorConstants.h"
#include "NodeRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>

#include <imgui.h>

#include "Application.h"
#include "VisionCraftApplication.h"

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/PreviewNode.h"
#include "Nodes/ThresholdNode.h"


namespace VisionCraft
{
    NodeEditorLayer::NodeEditorLayer() : nodeRenderer(canvas, connectionManager)
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
        auto &io = ImGui::GetIO();
        const auto mousePos = io.MousePos;

        // Handle Delete key for selected nodes
        if (selectionManager.HasSelection() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            std::vector<Engine::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                DeleteNode(nodeId);
            }
            selectionManager.ClearSelection();
            return;
        }

        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            hoveredConnection = std::nullopt;
            return;
        }

        // Update hovered connection for visual feedback (only if not dragging or interacting with pins)
        if (!selectionManager.IsDragging() && hoveredPin.nodeId == Constants::Special::kInvalidNodeId)
        {
            auto &nodeEditor = GetNodeEditor();
            hoveredConnection = connectionManager.FindConnectionAtPosition(mousePos, nodeEditor, nodePositions, canvas);
        }
        else
        {
            hoveredConnection = std::nullopt;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            // First check if clicking on a connection (reuse hoveredConnection)
            if (hoveredConnection.has_value())
            {
                // Right-click on connection - delete it
                connectionManager.RemoveConnection(hoveredConnection.value());
                hoveredConnection = std::nullopt;
                return;
            }

            // Then check if clicking on a node
            const auto clickedNodeId = FindNodeAtPosition(mousePos);
            if (clickedNodeId == Constants::Special::kInvalidNodeId)
            {
                // Right-click on empty space - clear selection and show creation menu
                selectionManager.ClearSelection();
                showContextMenu = true;
                contextMenuPos = mousePos;
                contextMenuRenderer.Open();
            }
            else
            {
                // Right-click on node - select it and show context menu
                if (!selectionManager.IsNodeSelected(clickedNodeId))
                {
                    selectionManager.SelectNode(clickedNodeId);
                }
                contextMenuRenderer.Open();
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (io.WantCaptureMouse && ImGui::IsAnyItemActive())
            {
                return;
            }

            const auto clickedNodeId = FindNodeAtPosition(mousePos);
            const bool shiftPressed = io.KeyShift;

            if (clickedNodeId != Constants::Special::kInvalidNodeId)
            {
                // Clicked on a node
                if (shiftPressed)
                {
                    // Shift+click: toggle selection
                    selectionManager.ToggleNodeSelection(clickedNodeId);
                }
                else
                {
                    // Normal click: select only this node (unless already in multi-selection)
                    if (!selectionManager.IsNodeSelected(clickedNodeId))
                    {
                        selectionManager.SelectNode(clickedNodeId);
                    }
                }

                // Start dragging all selected nodes
                // Convert nodePositions to screen positions for drag calculation
                std::unordered_map<Engine::NodeId, ImVec2> screenPositions;
                for (const auto nodeId : selectionManager.GetSelectedNodes())
                {
                    const auto &nodePos = nodePositions[nodeId];
                    screenPositions[nodeId] = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
                }
                selectionManager.StartDrag(mousePos, screenPositions);
            }
            else
            {
                // Clicked on empty space
                if (!shiftPressed)
                {
                    // Start box selection
                    selectionManager.StartBoxSelection(mousePos);
                }
            }
        }

        // Handle box selection
        if (selectionManager.IsBoxSelecting() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            selectionManager.UpdateBoxSelection(mousePos);
            UpdateBoxSelection();
        }

        if (selectionManager.IsBoxSelecting() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            selectionManager.EndBoxSelection();
        }

        // Handle node dragging
        if (selectionManager.IsDragging() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const auto &dragOffsets = selectionManager.GetDragOffsets();
            for (const auto &[nodeId, offset] : dragOffsets)
            {
                const auto newWorldPos = ImVec2(mousePos.x - offset.x, mousePos.y - offset.y);
                const auto newNodePos = canvas.ScreenToWorld(newWorldPos);
                nodePositions[nodeId] = NodePosition{ newNodePos.x, newNodePos.y };
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            selectionManager.StopDrag();
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
        const auto result = contextMenuRenderer.Render(selectionManager.HasSelection());

        switch (result.action)
        {
        case ContextMenuResult::Action::DeleteNodes: {
            std::vector<Engine::NodeId> nodesToDelete(
                selectionManager.GetSelectedNodes().begin(), selectionManager.GetSelectedNodes().end());
            for (const auto nodeId : nodesToDelete)
            {
                DeleteNode(nodeId);
            }
            selectionManager.ClearSelection();
            break;
        }
        case ContextMenuResult::Action::CreateNode:
            CreateNodeAtPosition(result.nodeType, contextMenuPos);
            break;
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

        // Use factory to create node
        auto newNode = nodeFactory.Create(nodeType, nodeId, displayName);

        if (newNode)
        {
            auto &nodeEditor = GetNodeEditor();

            const auto actualNodeId = newNode->GetId();
            nodeEditor.AddNode(std::move(newNode));
            nodePositions[actualNodeId] = { worldX, worldY };

            selectionManager.ClearSelection();
        }
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
} // namespace VisionCraft