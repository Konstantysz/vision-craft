#include "ConnectionManager.h"
#include "NodeEditorLayer.h"
#include "NodeRenderer.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>

namespace VisionCraft
{
    ConnectionManager::ConnectionManager()
    {
    }

    ConnectionManager::~ConnectionManager()
    {
    }

    void ConnectionManager::HandleConnectionInteractions(Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas)
    {
        const auto &io = ImGui::GetIO();
        const auto mousePos = io.MousePos;
        if (!ImGui::IsWindowHovered() || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            return;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (io.WantCaptureMouse && ImGui::IsAnyItemActive())
            {
                return;
            }

            const auto clickedPin = FindPinAtPosition(mousePos, nodeEditor, nodePositions, canvas);
            if (clickedPin.nodeId != Constants::Special::kInvalidNodeId)
            {
                if (!connectionState.isCreating)
                {
                    connectionState.isCreating = true;
                    connectionState.startPin = clickedPin;
                    connectionState.endPosition = mousePos;
                }
                else
                {
                    const auto &startPin = connectionState.startPin;
                    const auto &endPin = clickedPin;
                    const auto *startNode = nodeEditor.GetNode(startPin.nodeId);
                    const auto *endNode = nodeEditor.GetNode(endPin.nodeId);

                    if (startNode && endNode)
                    {
                        const auto startPins = GetNodePins(startNode->GetName());
                        const auto endPins = GetNodePins(endNode->GetName());
                        auto startIsOutput = false;
                        auto endIsInput = false;

                        for (const auto &pin : startPins)
                        {
                            if (pin.name == startPin.pinName && !pin.isInput)
                            {
                                startIsOutput = true;
                                break;
                            }
                        }

                        for (const auto &pin : endPins)
                        {
                            if (pin.name == endPin.pinName && pin.isInput)
                            {
                                endIsInput = true;
                                break;
                            }
                        }

                        if (startIsOutput && endIsInput)
                        {
                            CreateConnection(startPin, endPin, nodeEditor);
                        }
                        else if (!startIsOutput && !endIsInput)
                        {
                            CreateConnection(endPin, startPin, nodeEditor);
                        }
                    }

                    connectionState.isCreating = false;
                }
            }
            else if (connectionState.isCreating)
            {
                connectionState.isCreating = false;
            }
        }

        if (connectionState.isCreating)
        {
            connectionState.endPosition = mousePos;
        }

        if (connectionState.isCreating && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            connectionState.isCreating = false;
        }
    }

    void ConnectionManager::RenderConnections(const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        for (const auto &connection : connections)
        {
            RenderConnection(connection, nodeEditor, nodePositions, canvas);
        }

        if (connectionState.isCreating)
        {
            const auto startPos = GetPinWorldPosition(connectionState.startPin, nodeEditor, nodePositions, canvas);
            const auto endPos = connectionState.endPosition;
            const auto connectionColor = Constants::Colors::Connection::kCreating;
            const auto connectionThickness = Constants::Connection::kThickness;

            drawList->AddLine(startPos, endPos, connectionColor, connectionThickness);
        }
    }

    bool ConnectionManager::CreateConnection(const PinId &outputPin,
        const PinId &inputPin,
        Engine::NodeEditor &nodeEditor)
    {
        if (!IsConnectionValid(outputPin, inputPin, nodeEditor))
        {
            return false;
        }

        RemoveConnectionToInput(inputPin);
        connections.push_back({ outputPin, inputPin });
        nodeEditor.AddConnection(outputPin.nodeId, inputPin.nodeId);

        return true;
    }

    bool ConnectionManager::IsConnectionValid(const PinId &outputPin,
        const PinId &inputPin,
        const Engine::NodeEditor &nodeEditor) const
    {
        if (outputPin.nodeId == inputPin.nodeId)
        {
            return false;
        }

        const auto *outputNode = nodeEditor.GetNode(outputPin.nodeId);
        const auto *inputNode = nodeEditor.GetNode(inputPin.nodeId);
        if (!outputNode || !inputNode)
        {
            return false;
        }

        const auto outputPins = GetNodePins(outputNode->GetName());
        const auto inputPins = GetNodePins(inputNode->GetName());
        const auto outputPinIt = std::find_if(outputPins.begin(), outputPins.end(), [&outputPin](const auto &pin) {
            return pin.name == outputPin.pinName;
        });

        const auto inputPinIt = std::find_if(
            inputPins.begin(), inputPins.end(), [&inputPin](const auto &pin) { return pin.name == inputPin.pinName; });
        if (outputPinIt == outputPins.end() || inputPinIt == inputPins.end())
        {
            return false;
        }

        const auto &actualOutputPin = *outputPinIt;
        const auto &actualInputPin = *inputPinIt;
        if (actualOutputPin.isInput || !actualInputPin.isInput)
        {
            return false;
        }

        if (actualOutputPin.dataType != actualInputPin.dataType)
        {
            return false;
        }

        return true;
    }

    bool ConnectionManager::IsPinConnected(const PinId &pin) const
    {
        for (const auto &connection : connections)
        {
            if (connection.inputPin == pin || connection.outputPin == pin)
            {
                return true;
            }
        }
        return false;
    }

    bool ConnectionManager::PinNeedsInputWidget(Engine::NodeId nodeId, const NodePin &pin) const
    {
        return pin.isInput && pin.dataType != PinDataType::Image && !IsPinConnected({ nodeId, pin.name });
    }

    PinId ConnectionManager::FindPinAtPosition(const ImVec2 &mousePos,
        const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        PinId closestPin = { Constants::Special::kInvalidNodeId, "" };
        float closestDistanceSquared = std::numeric_limits<float>::max();

        const auto nodeIds = nodeEditor.GetNodeIds();
        for (const auto nodeId : nodeIds)
        {
            const auto *node = nodeEditor.GetNode(nodeId);
            if (!node || nodePositions.find(nodeId) == nodePositions.end())
            {
                continue;
            }

            const auto pins = GetNodePins(node->GetName());
            const auto dimensions = NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
            const auto &nodePos = nodePositions.at(nodeId);
            const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

            const auto pinInNode = FindPinAtPositionInNode(mousePos, nodeId, nodeEditor, nodePositions, canvas);
            if (pinInNode.nodeId != Constants::Special::kInvalidNodeId)
            {
                return pinInNode;
            }
        }

        return closestPin;
    }

    PinId ConnectionManager::FindPinAtPositionInNode(const ImVec2 &mousePos,
        Engine::NodeId nodeId,
        const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        const auto *node = nodeEditor.GetNode(nodeId);
        if (!node || nodePositions.find(nodeId) == nodePositions.end())
        {
            return { Constants::Special::kInvalidNodeId, "" };
        }

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        std::vector<NodePin> inputPins, outputPins;
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) { return pin.isInput; });
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();
        const auto pinRadius = Constants::Pin::kRadius * canvas.GetZoomLevel();

        // Check input pins using dynamic spacing
        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto startY = nodeWorldPos.y + titleHeight + padding;
        float currentY = startY;

        for (std::size_t i = 0; i < inputPins.size(); ++i)
        {
            const auto &pin = inputPins[i];
            const bool needsInputWidget = PinNeedsInputWidget(nodeId, pin);
            const auto currentPinHeight = needsInputWidget ? extendedPinHeight : compactPinHeight;
            const auto currentSpacing = needsInputWidget ? normalSpacing : compactSpacing;

            const auto pinPos = ImVec2(leftColumnX, currentY + currentPinHeight * 0.5f);
            const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
            const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
            if (distanceSquared <= pinRadius * pinRadius)
            {
                return { nodeId, pin.name };
            }

            currentY += currentPinHeight + currentSpacing;
        }

        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        currentY = startY;

        for (std::size_t i = 0; i < outputPins.size(); ++i)
        {
            const auto &pin = outputPins[i];
            const auto currentPinHeight = compactPinHeight;
            const auto currentSpacing = compactSpacing;

            const auto pinPos = ImVec2(rightColumnX, currentY + currentPinHeight * 0.5f);
            const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
            const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
            if (distanceSquared <= pinRadius * pinRadius)
            {
                return { nodeId, pin.name };
            }

            currentY += currentPinHeight + currentSpacing;
        }

        return { Constants::Special::kInvalidNodeId, "" };
    }

    ImVec2 ConnectionManager::GetPinWorldPosition(const PinId &pinId,
        const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        if (pinId.nodeId == Constants::Special::kInvalidNodeId
            || nodePositions.find(pinId.nodeId) == nodePositions.end())
        {
            return ImVec2(0, 0);
        }

        const auto *node = nodeEditor.GetNode(pinId.nodeId);
        if (!node)
        {
            return ImVec2(0, 0);
        }

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(pinId.nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();

        std::vector<NodePin> inputPins, outputPins;
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) { return pin.isInput; });
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        const auto startY = nodeWorldPos.y + titleHeight + padding;

        float currentY = startY;
        for (const auto &pin : inputPins)
        {
            if (pin.name == pinId.pinName)
            {
                const bool needsInputWidget = PinNeedsInputWidget(pinId.nodeId, pin);
                const auto currentPinHeight = needsInputWidget ? extendedPinHeight : compactPinHeight;
                return ImVec2(leftColumnX, currentY + currentPinHeight * 0.5f);
            }

            const bool needsInputWidget = PinNeedsInputWidget(pinId.nodeId, pin);
            const auto currentPinHeight = needsInputWidget ? extendedPinHeight : compactPinHeight;
            const auto currentSpacing = needsInputWidget ? normalSpacing : compactSpacing;
            currentY += currentPinHeight + currentSpacing;
        }

        currentY = startY;
        for (const auto &pin : outputPins)
        {
            if (pin.name == pinId.pinName)
            {
                return ImVec2(rightColumnX, currentY + compactPinHeight * 0.5f);
            }
            currentY += compactPinHeight + compactSpacing;
        }

        return ImVec2(0, 0);
    }

    const std::vector<NodeConnection> &ConnectionManager::GetConnections() const
    {
        return connections;
    }

    const ConnectionState &ConnectionManager::GetConnectionState() const
    {
        return connectionState;
    }

    std::vector<NodePin> ConnectionManager::GetNodePins(const std::string &nodeName)
    {
        static const std::unordered_map<std::string, std::vector<NodePin>> nodePinDefinitions = {
            { "Image Input", { { "filepath", PinDataType::Path, true }, { "Output", PinDataType::Image, false } } },
            { "Image Output",
                { { "Input", PinDataType::Image, true },
                    { "savePath", PinDataType::Path, true },
                    { "autoSave", PinDataType::Bool, true },
                    { "format", PinDataType::String, true } } },
            { "Grayscale",
                { { "Input", PinDataType::Image, true },
                    { "method", PinDataType::String, true },
                    { "preserveAlpha", PinDataType::Bool, true },
                    { "Output", PinDataType::Image, false } } },
            { "Canny Edge",
                { { "Input", PinDataType::Image, true },
                    { "lowThreshold", PinDataType::Float, true },
                    { "highThreshold", PinDataType::Float, true },
                    { "apertureSize", PinDataType::Int, true },
                    { "l2Gradient", PinDataType::Bool, true },
                    { "Output", PinDataType::Image, false } } },
            { "Threshold",
                { { "Input", PinDataType::Image, true },
                    { "threshold", PinDataType::Float, true },
                    { "maxValue", PinDataType::Float, true },
                    { "type", PinDataType::String, true },
                    { "Output", PinDataType::Image, false } } },
            { "Preview", { { "Input", PinDataType::Image, true }, { "Output", PinDataType::Image, false } } }
        };

        auto it = nodePinDefinitions.find(nodeName);
        if (it != nodePinDefinitions.end())
        {
            return it->second;
        }

        return {};
    }

    bool ConnectionManager::IsCreatingConnection() const
    {
        return connectionState.isCreating;
    }

    const PinId &ConnectionManager::GetStartPin() const
    {
        return connectionState.startPin;
    }

    void ConnectionManager::RemoveConnectionToInput(const PinId &inputPin)
    {
        connections.erase(std::remove_if(connections.begin(),
                              connections.end(),
                              [&inputPin](const NodeConnection &conn) { return conn.inputPin == inputPin; }),
            connections.end());
    }

    void ConnectionManager::RenderConnection(const NodeConnection &connection,
        const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        const auto startPos = GetPinWorldPosition(connection.outputPin, nodeEditor, nodePositions, canvas);
        const auto endPos = GetPinWorldPosition(connection.inputPin, nodeEditor, nodePositions, canvas);

        if (startPos.x == 0 && startPos.y == 0)
        {
            return;
        }

        if (endPos.x == 0 && endPos.y == 0)
        {
            return;
        }

        const auto connectionColor = Constants::Colors::Connection::kActive;
        const auto connectionThickness = Constants::Connection::kThickness;
        const auto bezierTension = Constants::Connection::kBezierTension;
        const auto distance = std::abs(endPos.x - startPos.x);
        const auto tension = std::min(distance * 0.5f, bezierTension * canvas.GetZoomLevel());
        const auto cp1 = ImVec2(startPos.x + tension, startPos.y);
        const auto cp2 = ImVec2(endPos.x - tension, endPos.y);
        drawList->AddBezierCubic(startPos, cp1, cp2, endPos, connectionColor, connectionThickness);
    }
} // namespace VisionCraft