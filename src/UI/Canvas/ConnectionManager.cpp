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
                        const auto startPins = GetNodePins(startNode);
                        const auto endPins = GetNodePins(endNode);
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

        // Determine connection type based on pin type (execution vs data)
        const auto *outputNode = nodeEditor.GetNode(outputPin.nodeId);
        Nodes::ConnectionType connectionType = Nodes::ConnectionType::Data;
        bool isExecutionConnection = false;

        if (outputNode)
        {
            // Check if this is an execution pin
            if (outputNode->HasExecutionOutputPin(outputPin.pinName))
            {
                connectionType = Nodes::ConnectionType::Execution;
                isExecutionConnection = true;
            }
        }

        // For execution pins: remove BOTH existing connections (1:1 rule)
        // For data pins: remove only connection to input (1:N rule - one input, many outputs)
        if (isExecutionConnection)
        {
            // Remove any existing connection FROM this execution output pin (1:1)
            RemoveConnectionFromOutput(outputPin);
        }
        RemoveConnectionToInput(inputPin);

        connections.push_back(newConnection);
        nodeEditor.AddConnection(
            outputPin.nodeId, outputPin.pinName, inputPin.nodeId, inputPin.pinName, connectionType);

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

        const auto outputPins = GetNodePins(outputNode);
        const auto inputPins = GetNodePins(inputNode);
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

        // Verify pin directions (output -> input only)
        if (actualOutputPin.isInput || !actualInputPin.isInput)
        {
            return false;
        }

        // Pin types must match (execution <-> execution, data <-> data)
        if (actualOutputPin.pinType != actualInputPin.pinType)
        {
            return false;
        }

        // For data pins, data types must match
        // For execution pins, data type check is not needed
        if (actualOutputPin.pinType == Widgets::PinType::Data)
        {
            if (actualOutputPin.dataType != actualInputPin.dataType)
            {
                return false;
            }
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
        return pin.isInput && pin.pinType != Widgets::PinType::Execution // Execution pins never need input widgets
               && pin.dataType != Widgets::PinDataType::Image            // Image pins don't have widgets either
               && !IsPinConnected({ nodeId, pin.name });
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

            const auto pins = GetNodePins(node);
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

        const auto pins = GetNodePins(node);
        const auto dimensions = Rendering::NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        // Separate pins into execution and data pins
        const auto [executionInputPins, executionOutputPins, dataInputPins, dataOutputPins] =
            Widgets::SeparatePinsByType(pins);

        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();
        const auto pinRadius = Constants::Pin::kRadius * canvas.GetZoomLevel();

        const bool hasExecutionPins = !executionInputPins.empty() || !executionOutputPins.empty();
        const auto executionRowHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();

        // Check execution input pins (horizontal row at top left)
        if (hasExecutionPins)
        {
            const auto executionRowY = nodeWorldPos.y + titleHeight + padding + (executionRowHeight * 0.5f);

            for (const auto &pin : executionInputPins)
            {
                const auto pinPos = ImVec2(nodeWorldPos.x + padding, executionRowY);
                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }

            // Check execution output pins (horizontal row at top right)
            for (const auto &pin : executionOutputPins)
            {
                const auto pinPos = ImVec2(nodeWorldPos.x + dimensions.size.x - padding, executionRowY);
                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }
        }

        // Check data input pins using dynamic spacing (column below execution row)
        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto executionRowOffset = hasExecutionPins ? (executionRowHeight + compactSpacing) : 0.0f;
        const auto startY = nodeWorldPos.y + titleHeight + padding + executionRowOffset;
        float currentY = startY;

        for (std::size_t i = 0; i < dataInputPins.size(); ++i)
        {
            const auto &pin = dataInputPins[i];
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

        // Check data output pins (column below execution row)
        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        currentY = startY;

        for (std::size_t i = 0; i < dataOutputPins.size(); ++i)
        {
            const auto &pin = dataOutputPins[i];
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

        const auto pins = GetNodePins(node);
        const auto dimensions = Rendering::NodeRenderer::CalculateNodeDimensions(pins, canvas.GetZoomLevel(), node);
        const auto &nodePos = nodePositions.at(pinId.nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();

        // Separate pins into execution and data pins
        const auto [executionInputPins, executionOutputPins, dataInputPins, dataOutputPins] =
            Widgets::SeparatePinsByType(pins);

        const bool hasExecutionPins = !executionInputPins.empty() || !executionOutputPins.empty();
        const auto executionRowHeight = Constants::Pin::kCompactHeight * canvas.GetZoomLevel();

        // Check if requested pin is an execution input pin (horizontal row at top left)
        if (hasExecutionPins)
        {
            const auto executionRowY = nodeWorldPos.y + titleHeight + padding + (executionRowHeight * 0.5f);

            for (const auto &pin : executionInputPins)
            {
                if (pin.name == pinId.pinName)
                {
                    return ImVec2(nodeWorldPos.x + padding, executionRowY);
                }
            }

            // Check if requested pin is an execution output pin (horizontal row at top right)
            for (const auto &pin : executionOutputPins)
            {
                if (pin.name == pinId.pinName)
                {
                    return ImVec2(nodeWorldPos.x + dimensions.size.x - padding, executionRowY);
                }
            }
        }

        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        const auto executionRowOffset = hasExecutionPins ? (executionRowHeight + compactSpacing) : 0.0f;
        const auto startY = nodeWorldPos.y + titleHeight + padding + executionRowOffset;

        // Check data input pins
        float currentY = startY;
        for (const auto &pin : dataInputPins)
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

        // Check data output pins
        currentY = startY;
        for (const auto &pin : dataOutputPins)
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

    std::vector<Widgets::NodePin> ConnectionManager::GetNodePins(const Nodes::Node *node)
    {
        if (!node)
        {
            return {};
        }

        const std::string &nodeName = node->GetName();

        static const std::unordered_map<std::string, std::vector<Widgets::NodePin>> nodePinDefinitions = {
            { "Image Input",
                { { "FilePath", Widgets::PinType::Data, Widgets::PinDataType::Path, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Image Output",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "SavePath", Widgets::PinType::Data, Widgets::PinDataType::Path, true },
                    { "AutoSave", Widgets::PinType::Data, Widgets::PinDataType::Bool, true },
                    { "Format", Widgets::PinType::Data, Widgets::PinDataType::String, true } } },
            { "Grayscale",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Method", Widgets::PinType::Data, Widgets::PinDataType::String, true },
                    { "PreserveAlpha", Widgets::PinType::Data, Widgets::PinDataType::Bool, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Canny Edge",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "LowThreshold", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "HighThreshold", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "ApertureSize", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "L2Gradient", Widgets::PinType::Data, Widgets::PinDataType::Bool, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Threshold",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Threshold", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "MaxValue", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "Type", Widgets::PinType::Data, Widgets::PinDataType::String, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Preview",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Sobel Edge Detection",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "dx", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "dy", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "ksize", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "scale", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "delta", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Convert Color",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Conversion", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Split Channels",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Channel 1", Widgets::PinType::Data, Widgets::PinDataType::Image, false },
                    { "Channel 2", Widgets::PinType::Data, Widgets::PinDataType::Image, false },
                    { "Channel 3", Widgets::PinType::Data, Widgets::PinDataType::Image, false },
                    { "Channel 4", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Merge Channels",
                { { "Channel 1", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Channel 2", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Channel 3", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Channel 4", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Median Blur",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "ksize", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Morphology",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Operation", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "ksize", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "iterations", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            { "Resize",
                { { "Input", Widgets::PinType::Data, Widgets::PinDataType::Image, true },
                    { "Width", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "Height", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "ScaleX", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "ScaleY", Widgets::PinType::Data, Widgets::PinDataType::Float, true },
                    { "Interpolation", Widgets::PinType::Data, Widgets::PinDataType::Int, true },
                    { "Output", Widgets::PinType::Data, Widgets::PinDataType::Image, false } } },
            // Execution flow nodes - pins are dynamically queried from Node, but we still need entries for proper
            // rendering
            { "BeginPlay", {} },
            { "Sequence", {} }
        };

        std::vector<Widgets::NodePin> pins;

        // Add execution input pins first (at the top)
        const auto executionInputPins = node->GetExecutionInputPins();
        for (const auto &pinName : executionInputPins)
        {
            pins.push_back({ pinName, Widgets::PinType::Execution, Widgets::PinDataType::Execution, true });
        }

        // Add data pins from static definitions
        auto it = nodePinDefinitions.find(nodeName);
        if (it != nodePinDefinitions.end())
        {
            pins.insert(pins.end(), it->second.begin(), it->second.end());
        }

        // Add execution output pins last (at the bottom)
        const auto executionOutputPins = node->GetExecutionOutputPins();
        for (const auto &pinName : executionOutputPins)
        {
            pins.push_back({ pinName, Widgets::PinType::Execution, Widgets::PinDataType::Execution, false });
        }

        return pins;
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

    void ConnectionManager::RemoveConnectionFromOutput(const Widgets::PinId &outputPin)
    {
        connections.erase(
            std::remove_if(connections.begin(),
                connections.end(),
                [&outputPin](const Widgets::NodeConnection &conn) { return conn.outputPin == outputPin; }),
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
