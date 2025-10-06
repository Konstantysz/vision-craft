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

            selectedNodeId = Constants::Special::kInvalidNodeId;
            isDragging = false;
            dragOffset = {};
        }

        HandleMouseInteractions();
        DetectHoveredPin();
        connectionManager.HandleConnectionInteractions(nodeEditor, nodePositions, canvas);

        connectionManager.RenderConnections(nodeEditor, nodePositions, canvas);
        RenderNodes();

        RenderContextMenu();

        canvas.EndCanvas();

        ImGui::End();

        // Render file dialogs
        if (showSaveDialog)
        {
            ImGui::OpenPopup("Save Graph");
            RenderSaveDialog();
        }

        if (showLoadDialog)
        {
            ImGui::OpenPopup("Load Graph");
            RenderLoadDialog();
        }
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

        nodeRenderer.RenderNode(node, nodePos, selectedNodeId, getPinInteractionState);
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

        // Handle Delete key for selected node
        if (selectedNodeId != Constants::Special::kInvalidNodeId && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            DeleteNode(selectedNodeId);
            return;
        }

        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            return;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            // First check if clicking on a connection
            auto &nodeEditor = GetNodeEditor();
            auto connection = connectionManager.FindConnectionAtPosition(mousePos, nodeEditor, nodePositions, canvas);
            if (connection.has_value())
            {
                // Right-click on connection - delete it
                connectionManager.RemoveConnection(connection.value());
                return;
            }

            // Then check if clicking on a node
            const auto clickedNodeId = FindNodeAtPosition(mousePos);
            if (clickedNodeId == Constants::Special::kInvalidNodeId)
            {
                showContextMenu = true;
                contextMenuPos = mousePos;
                ImGui::OpenPopup("NodeContextMenu");
            }
            else
            {
                // Right-click on node - select it and show context menu
                selectedNodeId = clickedNodeId;
                ImGui::OpenPopup("NodeContextMenu");
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (io.WantCaptureMouse && ImGui::IsAnyItemActive())
            {
                return;
            }

            const auto clickedNodeId = FindNodeAtPosition(mousePos);
            if (clickedNodeId != Constants::Special::kInvalidNodeId)
            {
                selectedNodeId = clickedNodeId;
                isDragging = true;

                const auto &nodePos = nodePositions[clickedNodeId];
                const auto worldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
                dragOffset = ImVec2(mousePos.x - worldPos.x, mousePos.y - worldPos.y);
            }
            else
            {
                selectedNodeId = Constants::Special::kInvalidNodeId;
                isDragging = false;
            }
        }

        if (isDragging && selectedNodeId != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const auto newWorldPos = ImVec2(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);
            const auto newNodePos = canvas.ScreenToWorld(newWorldPos);
            nodePositions[selectedNodeId] = NodePosition{ newNodePos.x, newNodePos.y };
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            isDragging = false;
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
        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            // If a node is selected, show node operations
            if (selectedNodeId != Constants::Special::kInvalidNodeId)
            {
                if (ImGui::MenuItem("Delete"))
                {
                    DeleteNode(selectedNodeId);
                    ImGui::CloseCurrentPopup();
                }
            }
            else
            {
                // Otherwise show node creation menu
                ImGui::Text("Add Node");
                ImGui::Separator();

                if (ImGui::MenuItem("Image Input"))
                {
                    CreateNodeAtPosition("ImageInput", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Image Output"))
                {
                    CreateNodeAtPosition("ImageOutput", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Preview"))
                {
                    CreateNodeAtPosition("Preview", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Grayscale"))
                {
                    CreateNodeAtPosition("Grayscale", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Canny Edge Detection"))
                {
                    CreateNodeAtPosition("CannyEdge", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Threshold"))
                {
                    CreateNodeAtPosition("Threshold", contextMenuPos);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    void NodeEditorLayer::CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position)
    {
        const auto worldPos = canvas.ScreenToWorld(position);
        auto worldX = worldPos.x;
        auto worldY = worldPos.y;

        worldX -= Constants::Node::Creation::kOffsetX;
        worldY -= Constants::Node::Creation::kOffsetY;

        const auto nodeId = nextNodeId++;
        std::unique_ptr<Engine::Node> newNode;
        if (nodeType == "ImageInput")
        {
            newNode = std::make_unique<Engine::ImageInputNode>(nodeId, "Image Input");
        }
        else if (nodeType == "ImageOutput")
        {
            newNode = std::make_unique<Engine::ImageOutputNode>(nodeId, "Image Output");
        }
        else if (nodeType == "Grayscale")
        {
            newNode = std::make_unique<Engine::GrayscaleNode>(nodeId, "Grayscale");
        }
        else if (nodeType == "CannyEdge")
        {
            newNode = std::make_unique<Engine::CannyEdgeNode>(nodeId, "Canny Edge");
        }
        else if (nodeType == "Threshold")
        {
            newNode = std::make_unique<Engine::ThresholdNode>(nodeId, "Threshold");
        }
        else if (nodeType == "Preview")
        {
            newNode = std::make_unique<Engine::PreviewNode>(nodeId, "Preview");
        }

        if (newNode)
        {
            auto &nodeEditor = GetNodeEditor();

            const auto actualNodeId = newNode->GetId();
            nodeEditor.AddNode(std::move(newNode));
            nodePositions[actualNodeId] = { worldX, worldY };

            selectedNodeId = Constants::Special::kInvalidNodeId;
            isDragging = false;
            dragOffset = {};
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
            showSaveDialog = true;
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
        showLoadDialog = true;
    }

    void NodeEditorLayer::HandleNewGraph()
    {
        GetNodeEditor().Clear();
        nodePositions.clear();
        currentFilePath.clear();
        selectedNodeId = Constants::Special::kInvalidNodeId;
        isDragging = false;
        nextNodeId = 1;
        LOG_INFO("Created new graph");
    }

    void NodeEditorLayer::RenderSaveDialog()
    {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Save Graph", &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filepath", filePathBuffer, sizeof(filePathBuffer));

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                std::string filepath = filePathBuffer;
                if (!filepath.empty())
                {
                    if (!filepath.ends_with(".json"))
                    {
                        filepath += ".json";
                    }

                    currentFilePath = filepath;

                    std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;
                    for (const auto &[id, pos] : nodePositions)
                    {
                        positions[id] = { pos.x, pos.y };
                    }

                    if (GetNodeEditor().SaveToFile(currentFilePath, positions))
                    {
                        LOG_INFO("Graph saved successfully to: {}", currentFilePath);
                        showSaveDialog = false;
                        std::memset(filePathBuffer, 0, sizeof(filePathBuffer));
                    }
                    else
                    {
                        LOG_ERROR("Failed to save graph");
                    }
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showSaveDialog = false;
                std::memset(filePathBuffer, 0, sizeof(filePathBuffer));
            }

            ImGui::EndPopup();
        }
    }

    void NodeEditorLayer::RenderLoadDialog()
    {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Load Graph", &showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter filename:");
            ImGui::InputText("##filepath", filePathBuffer, sizeof(filePathBuffer));

            if (ImGui::Button("Load", ImVec2(120, 0)))
            {
                std::string filepath = filePathBuffer;
                if (!filepath.empty())
                {
                    if (!filepath.ends_with(".json"))
                    {
                        filepath += ".json";
                    }

                    std::unordered_map<Engine::NodeId, std::pair<float, float>> positions;

                    if (GetNodeEditor().LoadFromFile(filepath, positions))
                    {
                        nodePositions.clear();
                        for (const auto &[id, pos] : positions)
                        {
                            nodePositions[id] = NodePosition{ pos.first, pos.second };
                        }

                        currentFilePath = filepath;
                        selectedNodeId = Constants::Special::kInvalidNodeId;
                        isDragging = false;

                        // Update nextNodeId to be higher than any loaded ID
                        Engine::NodeId maxId = 0;
                        for (const auto &id : GetNodeEditor().GetNodeIds())
                        {
                            if (id > maxId)
                                maxId = id;
                        }
                        nextNodeId = maxId + 1;

                        LOG_INFO("Graph loaded successfully from: {}", currentFilePath);
                        showLoadDialog = false;
                        std::memset(filePathBuffer, 0, sizeof(filePathBuffer));
                    }
                    else
                    {
                        LOG_ERROR("Failed to load graph from: {}", filepath);
                    }
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showLoadDialog = false;
                std::memset(filePathBuffer, 0, sizeof(filePathBuffer));
            }

            ImGui::EndPopup();
        }
    }

    void NodeEditorLayer::DeleteNode(Engine::NodeId nodeId)
    {
        if (nodeId == Constants::Special::kInvalidNodeId)
        {
            return;
        }

        auto &nodeEditor = GetNodeEditor();

        // Clear selection if deleting selected node
        if (selectedNodeId == nodeId)
        {
            selectedNodeId = Constants::Special::kInvalidNodeId;
            isDragging = false;
        }

        // Remove node (also removes connections)
        nodeEditor.RemoveNode(nodeId);

        // Remove node position
        nodePositions.erase(nodeId);
    }
} // namespace VisionCraft