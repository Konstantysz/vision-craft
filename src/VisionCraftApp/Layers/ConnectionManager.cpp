#include "ConnectionManager.h"
#include "NodeEditorLayer.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace VisionCraft
{
    ConnectionManager::ConnectionManager()
    {
    }

    ConnectionManager::~ConnectionManager()
    {
    }

    void ConnectionManager::HandleConnectionInteractions(const Engine::NodeEditor &nodeEditor,
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
        const Engine::NodeEditor &nodeEditor)
    {
        if (!IsConnectionValid(outputPin, inputPin, nodeEditor))
        {
            return false;
        }

        RemoveConnectionToInput(inputPin);

        connections.push_back({ outputPin, inputPin });

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

    PinId ConnectionManager::FindPinAtPosition(const ImVec2 &mousePos,
        const Engine::NodeEditor &nodeEditor,
        const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
        const CanvasController &canvas) const
    {
        const auto nodeIds = nodeEditor.GetNodeIds();
        for (const auto nodeId : nodeIds)
        {
            const auto *node = nodeEditor.GetNode(nodeId);
            if (!node || nodePositions.find(nodeId) == nodePositions.end())
            {
                continue;
            }

            const auto pins = GetNodePins(node->GetName());
            const auto dimensions = CalculateNodeDimensions(pins, canvas.GetZoomLevel());
            const auto &nodePos = nodePositions.at(nodeId);
            const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));

            std::vector<NodePin> inputPins;
            std::copy_if(pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) {
                return pin.isInput && pin.dataType == PinDataType::Image;
            });

            const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
            const auto pinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
            const auto pinSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
            const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();
            const auto pinRadius = Constants::Pin::kRadius * canvas.GetZoomLevel();
            const auto leftColumnX = nodeWorldPos.x + padding;
            const auto inputY = nodeWorldPos.y + titleHeight + padding;
            for (std::size_t i = 0; i < inputPins.size(); ++i)
            {
                const auto &pin = inputPins[i];
                const auto pinPos = ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }

            std::vector<NodePin> outputPins;
            std::copy_if(
                pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

            const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
            const auto outputY = nodeWorldPos.y + titleHeight + padding;

            for (std::size_t i = 0; i < outputPins.size(); ++i)
            {
                const auto &pin = outputPins[i];
                const auto pinPos = ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;
                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }
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
        const auto dimensions = CalculateNodeDimensions(pins, canvas.GetZoomLevel());
        const auto &nodePos = nodePositions.at(pinId.nodeId);
        const auto nodeWorldPos = canvas.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
        const auto titleHeight = Constants::Node::kTitleHeight * canvas.GetZoomLevel();
        const auto pinHeight = Constants::Pin::kHeight * canvas.GetZoomLevel();
        const auto pinSpacing = Constants::Pin::kSpacing * canvas.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas.GetZoomLevel();

        std::vector<NodePin> inputPins;
        for (const auto &pin : pins)
        {
            if (pin.isInput && pin.dataType == PinDataType::Image)
            {
                inputPins.push_back(pin);
            }
        }

        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto inputY = nodeWorldPos.y + titleHeight + padding;
        for (std::size_t i = 0; i < inputPins.size(); ++i)
        {
            if (inputPins[i].name == pinId.pinName)
            {
                return ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            }
        }

        std::vector<NodePin> outputPins;
        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        const auto outputY = nodeWorldPos.y + titleHeight + padding;
        for (std::size_t i = 0; i < outputPins.size(); ++i)
        {
            if (outputPins[i].name == pinId.pinName)
            {
                return ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            }
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

    std::vector<NodePin> ConnectionManager::GetNodePins(const std::string &nodeName)
    {
        static const std::unordered_map<std::string, std::vector<NodePin>> nodePinDefinitions = {
            { "Image Input",
                {
                    { "filepath", PinDataType::String, true }, // Parameter input
                    { "Image", PinDataType::Image, false }     // Data output
                } },
            { "Image Output",
                {
                    { "Image", PinDataType::Image, true },     // Data input
                    { "savePath", PinDataType::String, true }, // Parameter input
                    { "autoSave", PinDataType::Bool, true },   // Parameter input
                    { "format", PinDataType::String, true }    // Parameter input
                } },
            { "Grayscale",
                {
                    { "Image", PinDataType::Image, true },        // Data input
                    { "method", PinDataType::String, true },      // Parameter input
                    { "preserveAlpha", PinDataType::Bool, true }, // Parameter input
                    { "Image", PinDataType::Image, false }        // Data output
                } },
            { "Canny Edge",
                {
                    { "Image", PinDataType::Image, true },         // Data input
                    { "lowThreshold", PinDataType::Float, true },  // Parameter input
                    { "highThreshold", PinDataType::Float, true }, // Parameter input
                    { "apertureSize", PinDataType::Int, true },    // Parameter input
                    { "l2Gradient", PinDataType::Bool, true },     // Parameter input
                    { "Image", PinDataType::Image, false }         // Data output
                } },
            { "Threshold",
                {
                    { "Image", PinDataType::Image, true },     // Data input
                    { "threshold", PinDataType::Float, true }, // Parameter input
                    { "maxValue", PinDataType::Float, true },  // Parameter input
                    { "type", PinDataType::String, true },     // Parameter input
                    { "Image", PinDataType::Image, false }     // Data output
                } }
        };

        auto it = nodePinDefinitions.find(nodeName);
        if (it != nodePinDefinitions.end())
        {
            return it->second;
        }

        return {};
    }

    NodeDimensions ConnectionManager::CalculateNodeDimensions(const std::vector<NodePin> &pins, float zoomLevel)
    {
        std::vector<NodePin> inputPins, outputPins, parameterPins;

        std::copy_if(pins.begin(), pins.end(), std::back_inserter(inputPins), [](const auto &pin) {
            return pin.isInput && pin.dataType == PinDataType::Image;
        });

        std::copy_if(pins.begin(), pins.end(), std::back_inserter(parameterPins), [](const auto &pin) {
            return pin.isInput && pin.dataType != PinDataType::Image;
        });

        std::copy_if(
            pins.begin(), pins.end(), std::back_inserter(outputPins), [](const auto &pin) { return !pin.isInput; });

        const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        const auto pinHeight = Constants::Pin::kHeight * zoomLevel;
        const auto paramHeight = Constants::Parameter::kHeight * zoomLevel;
        const auto pinSpacing = Constants::Pin::kSpacing * zoomLevel;
        const auto padding = Constants::Node::kPadding * zoomLevel;
        const auto maxPins = std::max(inputPins.size(), outputPins.size());
        const auto pinsHeight = maxPins * (pinHeight + pinSpacing);
        const auto parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
        const auto contentHeight = pinsHeight + parametersHeight + padding * 2;
        const auto totalHeight = titleHeight + contentHeight + padding;

        return { ImVec2(Constants::Node::kWidth * zoomLevel,
                     std::max(Constants::Node::kMinHeight * zoomLevel, totalHeight)),
            inputPins.size(),
            outputPins.size(),
            parameterPins.size() };
    }
} // namespace VisionCraft