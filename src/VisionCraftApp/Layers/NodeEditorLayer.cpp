#include "NodeEditorLayer.h"
#include "NodeEditorConstants.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#include <imgui.h>

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/ThresholdNode.h"


namespace VisionCraft
{
    NodeEditorLayer::NodeEditorLayer() : nodeRenderer(canvas, connectionManager)
    {
    }
    void NodeEditorLayer::OnEvent(Core::Event &event)
    {
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
        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            return;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            const auto clickedNodeId = FindNodeAtPosition(mousePos);
            if (clickedNodeId == Constants::Special::kInvalidNodeId)
            {
                showContextMenu = true;
                contextMenuPos = mousePos;
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

        if (newNode)
        {
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
        const auto nodeIds = nodeEditor.GetNodeIds();

        for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
        {
            const auto nodeId = *it;
            const auto *node = nodeEditor.GetNode(nodeId);

            if (!node || nodePositions.find(nodeId) == nodePositions.end())
                continue;

            const auto pins = connectionManager.GetNodePins(node->GetName());
            const auto dimensions = connectionManager.CalculateNodeDimensions(pins, canvas.GetZoomLevel());

            if (IsMouseOverNode(mousePos, nodePositions.at(nodeId), dimensions.size))
            {
                return nodeId;
            }
        }

        return Constants::Special::kInvalidNodeId;
    }

} // namespace VisionCraft