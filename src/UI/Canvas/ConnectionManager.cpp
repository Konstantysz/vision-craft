#include "UI/Canvas/ConnectionManager.h"
#include "UI/Layers/NodeEditorLayer.h"
#include "UI/Rendering/NodeRenderer.h"
#include "Logger.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <optional>

namespace VisionCraft::UI::Canvas
{
    ConnectionManager::ConnectionManager() : connectionState{}
    {
    }

    ConnectionManager::~ConnectionManager()
    {
    }

    void ConnectionManager::HandleConnectionInteractions(Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
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

    void ConnectionManager::RenderConnections(const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
        const CanvasController &canvas,
        const std::optional<Widgets::NodeConnection> &hoveredConnection)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        for (const auto &connection : connections)
        {
            const bool isHovered = hoveredConnection.has_value() && hoveredConnection.value() == connection;
            RenderConnection(connection, nodeEditor, nodePositions, canvas, isHovered);
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

    bool ConnectionManager::CreateConnection(const Widgets::PinId &outputPin,
        const Widgets::PinId &inputPin,
        Nodes::NodeEditor &nodeEditor,
        bool notifyCallback)
    {
        if (!IsConnectionValid(outputPin, inputPin, nodeEditor))
        {
            return false;
        }

        const Widgets::NodeConnection newConnection{ outputPin, inputPin };

        // If callback is enabled, let the command handle the actual connection creation
        if (notifyCallback && onConnectionCreated)
        {
            // Remove existing connection to input first
            RemoveConnectionToInput(inputPin);
            // Notify callback BEFORE adding connection
            // The callback will create a command which will call this method again with notifyCallback=false
            onConnectionCreated(newConnection);
            return true;
        }

        // Direct connection creation (from command execution or when callback is disabled)
        RemoveConnectionToInput(inputPin);
        connections.push_back(newConnection);
        nodeEditor.AddConnection(outputPin.nodeId, outputPin.pinName, inputPin.nodeId, inputPin.pinName);

        return true;
    }

    bool ConnectionManager::IsConnectionValid(const Widgets::PinId &outputPin,
        const Widgets::PinId &inputPin,
        const Nodes::NodeEditor &nodeEditor) const
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

    bool ConnectionManager::IsPinConnected(const Widgets::PinId &pin) const
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

    bool ConnectionManager::PinNeedsInputWidget(Nodes::NodeId nodeId, const Widgets::NodePin &pin) const
    {
        return pin.isInput && pin.dataType != Widgets::PinDataType::Image && !IsPinConnected({ nodeId, pin.name });
    }

    Widgets::PinId ConnectionManager::FindPinAtPosition(const ImVec2 &mousePos,
        const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        Widgets::PinId closestPin = { Constants::Special::kInvalidNodeId, "" };
        // closestDistanceSquared was unused

        const auto nodeIds = nodeEditor.GetNodeIds();
        for (const auto nodeId : nodeIds)
        {
            const auto *node = nodeEditor.GetNode(nodeId);
            if (!node || nodePositions.find(nodeId) == nodePositions.end())
            {
                continue;
            }

            const auto pins = GetNodePins(node->GetName());
            // dimensions variable was unused here
            const auto &nodePos = nodePositions.at(nodeId);
            // nodeWorldPos was unused

            const auto pinInNode = FindPinAtPositionInNode(mousePos, nodeId, nodeEditor, nodePositions, canvas);
            if (pinInNode.nodeId != Constants::Special::kInvalidNodeId)
            {
                return pinInNode;
            }
        }

        return closestPin;
    }

    Widgets::PinId ConnectionManager::FindPinAtPositionInNode(const ImVec2 &mousePos,
        Nodes::NodeId nodeId,
        const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        const auto *node = nodeEditor.GetNode(nodeId);
        if (!node || nodePositions.find(nodeId) == nodePositions.end())
        {
            return { Constants::Special::kInvalidNodeId, "" };
        }

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = Rendering::NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        std::vector<Widgets::NodePin> inputPins, outputPins;
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

    ImVec2 ConnectionManager::GetPinWorldPosition(const Widgets::PinId &pinId,
        const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
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
        const auto dimensions = Rendering::NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(pinId.nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();

        std::vector<Widgets::NodePin> inputPins, outputPins;
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

    const std::vector<Widgets::NodeConnection> &ConnectionManager::GetConnections() const
    {
        return connections;
    }

    const Widgets::ConnectionState &ConnectionManager::GetConnectionState() const
    {
        return connectionState;
    }

    std::vector<Widgets::NodePin> ConnectionManager::GetNodePins(const std::string &nodeName)
    {
        static const std::unordered_map<std::string, std::vector<Widgets::NodePin>> nodePinDefinitions = {
            { "Image Input",
                { { "FilePath", Widgets::PinDataType::Path, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Image Output",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "SavePath", Widgets::PinDataType::Path, true },
                    { "AutoSave", Widgets::PinDataType::Bool, true },
                    { "Format", Widgets::PinDataType::String, true } } },
            { "Grayscale",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Method", Widgets::PinDataType::String, true },
                    { "PreserveAlpha", Widgets::PinDataType::Bool, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Canny Edge",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "LowThreshold", Widgets::PinDataType::Float, true },
                    { "HighThreshold", Widgets::PinDataType::Float, true },
                    { "ApertureSize", Widgets::PinDataType::Int, true },
                    { "L2Gradient", Widgets::PinDataType::Bool, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Threshold",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Threshold", Widgets::PinDataType::Float, true },
                    { "MaxValue", Widgets::PinDataType::Float, true },
                    { "Type", Widgets::PinDataType::String, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Preview",
                { { "Input", Widgets::PinDataType::Image, true }, { "Output", Widgets::PinDataType::Image, false } } },
            { "Sobel Edge Detection",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "dx", Widgets::PinDataType::Int, true },
                    { "dy", Widgets::PinDataType::Int, true },
                    { "ksize", Widgets::PinDataType::Int, true },
                    { "scale", Widgets::PinDataType::Float, true },
                    { "delta", Widgets::PinDataType::Float, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Convert Color",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Conversion", Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Split Channels",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Channel 1", Widgets::PinDataType::Image, false },
                    { "Channel 2", Widgets::PinDataType::Image, false },
                    { "Channel 3", Widgets::PinDataType::Image, false },
                    { "Channel 4", Widgets::PinDataType::Image, false } } },
            { "Merge Channels",
                { { "Channel 1", Widgets::PinDataType::Image, true },
                    { "Channel 2", Widgets::PinDataType::Image, true },
                    { "Channel 3", Widgets::PinDataType::Image, true },
                    { "Channel 4", Widgets::PinDataType::Image, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Median Blur",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "ksize", Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Morphology",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Operation", Widgets::PinDataType::Int, true },
                    { "ksize", Widgets::PinDataType::Int, true },
                    { "iterations", Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinDataType::Image, false } } },
            { "Resize",
                { { "Input", Widgets::PinDataType::Image, true },
                    { "Width", Widgets::PinDataType::Int, true },
                    { "Height", Widgets::PinDataType::Int, true },
                    { "ScaleX", Widgets::PinDataType::Float, true },
                    { "ScaleY", Widgets::PinDataType::Float, true },
                    { "Interpolation", Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinDataType::Image, false } } }
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

    const Widgets::PinId &ConnectionManager::GetStartPin() const
    {
        return connectionState.startPin;
    }

    void ConnectionManager::RemoveConnectionToInput(const Widgets::PinId &inputPin)
    {
        connections.erase(std::remove_if(connections.begin(),
                              connections.end(),
                              [&inputPin](const Widgets::NodeConnection &conn) { return conn.inputPin == inputPin; }),
            connections.end());
    }

    void ConnectionManager::RenderConnection(const Widgets::NodeConnection &connection,
        const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
        const CanvasController &canvas,
        bool isHovered)
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

        // Highlight hovered connections with brighter color and thicker line
        const auto connectionColor = isHovered ? IM_COL32(255, 255, 100, 255) : Constants::Colors::Connection::kActive;
        const auto connectionThickness =
            isHovered ? Constants::Connection::kThickness * 2.0f : Constants::Connection::kThickness;

        const auto bezierTension = Constants::Connection::kBezierTension;
        const auto distance = std::abs(endPos.x - startPos.x);
        const auto tension = std::min(distance * 0.5f, bezierTension * canvas.GetZoomLevel());
        const auto cp1 = ImVec2(startPos.x + tension, startPos.y);
        const auto cp2 = ImVec2(endPos.x - tension, endPos.y);
        drawList->AddBezierCubic(startPos, cp1, cp2, endPos, connectionColor, connectionThickness);
    }

    std::optional<Widgets::NodeConnection> ConnectionManager::FindConnectionAtPosition(const ImVec2 &mousePos,
        const Nodes::NodeEditor &nodeEditor,
        const std::unordered_map<Nodes::NodeId, Widgets::NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        const float clickThreshold = 10.0f; // Distance threshold for clicking on a connection

        for (const auto &connection : connections)
        {
            // Get start and end positions
            const auto startPos = GetPinWorldPosition(connection.outputPin, nodeEditor, nodePositions, canvas);
            const auto endPos = GetPinWorldPosition(connection.inputPin, nodeEditor, nodePositions, canvas);

            // Calculate bezier curve parameters (same as in RenderConnection)
            const auto distance = std::abs(endPos.x - startPos.x);
            const auto bezierTension = Constants::Connection::kBezierTension;
            const auto tension = std::min(distance * 0.5f, bezierTension * canvas.GetZoomLevel());
            const auto cp1 = ImVec2(startPos.x + tension, startPos.y);
            const auto cp2 = ImVec2(endPos.x - tension, endPos.y);

            // Check distance to bezier curve by sampling points
            const int samples = 20;
            for (int i = 0; i <= samples; ++i)
            {
                const float t = static_cast<float>(i) / samples;
                const float u = 1.0f - t;
                const float t2 = t * t;
                const float u2 = u * u;
                const float t3 = t2 * t;
                const float u3 = u2 * u;

                // Cubic bezier formula
                const ImVec2 point = ImVec2(u3 * startPos.x + 3 * u2 * t * cp1.x + 3 * u * t2 * cp2.x + t3 * endPos.x,
                    u3 * startPos.y + 3 * u2 * t * cp1.y + 3 * u * t2 * cp2.y + t3 * endPos.y);

                const float dx = mousePos.x - point.x;
                const float dy = mousePos.y - point.y;
                const float distSq = dx * dx + dy * dy;

                if (distSq < clickThreshold * clickThreshold)
                {
                    return connection;
                }
            }
        }

        return std::nullopt;
    }

    void ConnectionManager::RemoveConnection(const Widgets::NodeConnection &connection)
    {
        connections.erase(std::remove(connections.begin(), connections.end(), connection), connections.end());
    }

    void ConnectionManager::RemoveConnectionsForNode(Nodes::NodeId nodeId)
    {
        connections.erase(std::remove_if(connections.begin(),
                              connections.end(),
                              [nodeId](const Widgets::NodeConnection &conn) {
                                  return conn.outputPin.nodeId == nodeId || conn.inputPin.nodeId == nodeId;
                              }),
            connections.end());
    }

    void ConnectionManager::SetConnectionCreatedCallback(ConnectionCreatedCallback callback)
    {
        onConnectionCreated = std::move(callback);
    }
} // namespace VisionCraft::UI::Canvas
