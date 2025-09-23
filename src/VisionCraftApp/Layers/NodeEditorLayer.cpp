#include "NodeEditorLayer.h"
#include "NodeEditorConstants.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/ThresholdNode.h"

namespace
{

    VisionCraft::NodeDimensions CalculateNodeDimensions(const std::vector<VisionCraft::NodePin> &pins, float zoomLevel)
    {
        std::vector<VisionCraft::NodePin> inputPins, outputPins, parameterPins;

        for (const auto &pin : pins)
        {
            if (pin.isInput)
            {
                if (pin.dataType == VisionCraft::PinDataType::Image)
                {
                    inputPins.push_back(pin);
                }
                else
                {
                    parameterPins.push_back(pin);
                }
            }
            else
            {
                outputPins.push_back(pin);
            }
        }

        const auto titleHeight = VisionCraft::Constants::Node::kTitleHeight * zoomLevel;
        const auto pinHeight = VisionCraft::Constants::Pin::kHeight * zoomLevel;
        const auto paramHeight = VisionCraft::Constants::Parameter::kHeight * zoomLevel;
        const auto pinSpacing = VisionCraft::Constants::Pin::kSpacing * zoomLevel;
        const auto padding = VisionCraft::Constants::Node::kPadding * zoomLevel;

        const auto maxPins = std::max(inputPins.size(), outputPins.size());
        const auto pinsHeight = maxPins * (pinHeight + pinSpacing);
        const auto parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
        const auto contentHeight = pinsHeight + parametersHeight + padding * 2;
        const auto totalHeight = titleHeight + contentHeight + padding;

        return { ImVec2(VisionCraft::Constants::Node::kWidth * zoomLevel,
                       std::max(VisionCraft::Constants::Node::kMinHeight * zoomLevel, totalHeight)),
            inputPins.size(),
            outputPins.size(),
            parameterPins.size() };
    }
} // namespace

namespace VisionCraft
{
    void NodeEditorLayer::OnEvent(Core::Event &event)
    {
    }

    void NodeEditorLayer::OnUpdate(float deltaTime)
    {
    }

    void NodeEditorLayer::OnRender()
    {
        ImGui::Begin("Node Editor");

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();

        currentCanvasPos = canvas_pos;

        if (canvas_size.x < Constants::Canvas::kMinSize)
        {
            canvas_size.x = Constants::Canvas::kMinSize;
        }

        if (canvas_size.y < Constants::Canvas::kMinSize)
        {
            canvas_size.y = Constants::Canvas::kMinSize;
        }

        if (showGrid)
        {
            const auto grid_step = gridSize * zoomLevel;

            for (float x = fmodf(panX, grid_step); x < canvas_size.x; x += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y),
                    ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
                    Constants::Colors::Grid::kLines);
            }

            for (float y = fmodf(panY, grid_step); y < canvas_size.y; y += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y),
                    ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
                    Constants::Colors::Grid::kLines);
            }
        }

        ImGuiIO &io = ImGui::GetIO();
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            panX += io.MouseDelta.x;
            panY += io.MouseDelta.y;
        }

        if (ImGui::IsWindowHovered())
        {
            if (io.MouseWheel != 0.0f)
            {
                zoomLevel += io.MouseWheel * Constants::Zoom::kStep;
                zoomLevel = std::clamp(zoomLevel, Constants::Zoom::kMin, Constants::Zoom::kMax);
            }
        }

        if (nodeEditor.GetNodeIds().empty())
        {
            auto starterNode = std::make_unique<Engine::ImageInputNode>(nextNodeId++);
            Engine::NodeId nodeId = starterNode->GetId();
            nodeEditor.AddNode(std::move(starterNode));
            nodePositions[nodeId] = { 100.0f, 100.0f };

            selectedNodeId = Constants::Special::kInvalidNodeId;
            isDragging = false;
            dragOffset = ImVec2(0.0f, 0.0f);
        }

        HandleMouseInteractions();
        HandleConnectionInteractions();

        RenderConnections();
        RenderNodes();

        RenderContextMenu();

        ImGui::End();
    }

    void NodeEditorLayer::RenderNodes()
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        for (Engine::NodeId nodeId : nodeEditor.GetNodeIds())
        {
            Engine::Node *node = nodeEditor.GetNode(nodeId);
            if (node && nodePositions.find(nodeId) != nodePositions.end())
            {
                RenderNode(node, nodePositions[nodeId]);
            }
        }
    }

    void NodeEditorLayer::RenderNode(Engine::Node *node, const NodePosition &nodePos)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        const auto canvasPos = currentCanvasPos;

        const auto worldPos =
            ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = CalculateNodeDimensions(pins, zoomLevel);

        const auto isSelected = (node->GetId() == selectedNodeId);
        const auto borderColor = isSelected ? Constants::Colors::Node::kBorderSelected : Constants::Colors::Node::kBorderNormal;
        const auto borderThickness = isSelected ? Constants::Node::Border::kThicknessSelected : Constants::Node::Border::kThicknessNormal;
        const auto nodeRounding = Constants::Node::kRounding * zoomLevel;

        // Draw node background
        drawList->AddRectFilled(
            worldPos, ImVec2(worldPos.x + dimensions.size.x, worldPos.y + dimensions.size.y), Constants::Colors::Node::kBackground, nodeRounding);

        // Draw node border
        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + dimensions.size.x, worldPos.y + dimensions.size.y),
            borderColor,
            nodeRounding,
            0,
            borderThickness * zoomLevel);

        // Draw title background
        const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + dimensions.size.x, worldPos.y + titleHeight),
            Constants::Colors::Node::kTitle,
            nodeRounding,
            ImDrawFlags_RoundCornersTop);

        // Draw title text
        if (zoomLevel > Constants::Zoom::kMinForText)
        {
            const auto textPos = ImVec2(worldPos.x + Constants::Node::kPadding * zoomLevel, worldPos.y + Constants::Node::Text::kOffset * zoomLevel);
            drawList->AddText(textPos, Constants::Colors::Node::kText, node->GetName().c_str());
        }

        // Render pins
        RenderNodePins(pins, worldPos, dimensions, zoomLevel);

        // Render parameters if needed
        if (dimensions.parameterPinCount > 0 && zoomLevel > Constants::Zoom::kMinForText)
        {
            const auto parametersStartY = worldPos.y + titleHeight + (Constants::Node::kPadding * zoomLevel)
                                          + (std::max(dimensions.inputPinCount, dimensions.outputPinCount)
                                              * (Constants::Pin::kHeight + Constants::Pin::kSpacing) * zoomLevel)
                                          + (Constants::Node::kPadding * zoomLevel);
            RenderNodeParameters(node, ImVec2(worldPos.x, parametersStartY), dimensions.size);
        }
    }

    bool NodeEditorLayer::IsMouseOverNode(const ImVec2 &mousePos,
        const NodePosition &nodePos,
        const ImVec2 &nodeSize) const
    {
        ImVec2 canvasPos = currentCanvasPos;
        ImVec2 worldPos =
            ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));

        return mousePos.x >= worldPos.x && mousePos.x <= worldPos.x + nodeSize.x && mousePos.y >= worldPos.y
               && mousePos.y <= worldPos.y + nodeSize.y;
    }

    void NodeEditorLayer::HandleMouseInteractions()
    {
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

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
                const auto worldPos =
                    ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));
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
            ImVec2 newWorldPos = ImVec2(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);

            float oldX = nodePositions[selectedNodeId].x;
            float oldY = nodePositions[selectedNodeId].y;
            nodePositions[selectedNodeId].x = (newWorldPos.x - canvasPos.x - panX) / zoomLevel;
            nodePositions[selectedNodeId].y = (newWorldPos.y - canvasPos.y - panY) / zoomLevel;
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            isDragging = false;
        }
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
        ImVec2 canvasPos = currentCanvasPos;

        float worldX = (position.x - canvasPos.x - panX) / zoomLevel;
        float worldY = (position.y - canvasPos.y - panY) / zoomLevel;

        worldX -= Constants::Node::Creation::kOffsetX;
        worldY -= Constants::Node::Creation::kOffsetY;

        Engine::NodeId nodeId = nextNodeId++;
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
            Engine::NodeId actualNodeId = newNode->GetId();
            nodeEditor.AddNode(std::move(newNode));
            nodePositions[actualNodeId] = { worldX, worldY };

            selectedNodeId = Constants::Special::kInvalidNodeId;
            isDragging = false;
            dragOffset = ImVec2(0.0f, 0.0f);
        }
    }

    std::vector<NodePin> NodeEditorLayer::GetNodePins(const std::string &nodeName)
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
        default:
            return Constants::Colors::Pin::kDefault;
        }
    }

    void NodeEditorLayer::RenderPin(const NodePin &pin, const ImVec2 &position, float radius) const
    {
        auto *drawList = ImGui::GetWindowDrawList();
        const auto pinColor = GetDataTypeColor(pin.dataType);

        drawList->AddCircleFilled(position, radius, pinColor);
        drawList->AddCircle(position, radius, Constants::Colors::Pin::kBorder, 0, Constants::Pin::kBorderThickness);
    }

    void NodeEditorLayer::RenderNodePins(const std::vector<NodePin> &pins,
        const ImVec2 &nodeWorldPos,
        const NodeDimensions &dimensions,
        float zoomLevel)
    {
        auto *drawList = ImGui::GetWindowDrawList();

        std::vector<NodePin> inputPins, outputPins;
        for (const auto &pin : pins)
        {
            if (pin.isInput && pin.dataType == PinDataType::Image)
            {
                inputPins.push_back(pin);
            }
            else if (!pin.isInput)
            {
                outputPins.push_back(pin);
            }
        }

        const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        const auto pinHeight = Constants::Pin::kHeight * zoomLevel;
        const auto pinSpacing = Constants::Pin::kSpacing * zoomLevel;
        const auto padding = Constants::Node::kPadding * zoomLevel;
        const auto pinRadius = Constants::Pin::kRadius * zoomLevel;
        const auto textOffset = Constants::Pin::kTextOffset * zoomLevel;

        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;

        // Render input pins
        const auto inputY = nodeWorldPos.y + titleHeight + padding;
        for (size_t i = 0; i < inputPins.size(); ++i)
        {
            const auto &pin = inputPins[i];
            const auto pinPos = ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            const auto labelPos = ImVec2(leftColumnX + pinRadius + textOffset, inputY + i * (pinHeight + pinSpacing));

            RenderPin(pin, pinPos, pinRadius);

            if (zoomLevel > Constants::Zoom::kMinForText)
            {
                const auto displayName = FormatParameterName(pin.name);
                drawList->AddText(labelPos, Constants::Colors::Pin::kLabel, displayName.c_str());
            }
        }

        // Render output pins
        const auto outputY = nodeWorldPos.y + titleHeight + padding;
        for (size_t i = 0; i < outputPins.size(); ++i)
        {
            const auto &pin = outputPins[i];
            const auto displayName = FormatParameterName(pin.name);
            const auto textSize = ImGui::CalcTextSize(displayName.c_str());
            const auto pinPos = ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            const auto labelPos =
                ImVec2(rightColumnX - pinRadius - textSize.x - textOffset, outputY + i * (pinHeight + pinSpacing));

            RenderPin(pin, pinPos, pinRadius);

            if (zoomLevel > Constants::Zoom::kMinForText)
            {
                drawList->AddText(labelPos, Constants::Colors::Pin::kLabel, displayName.c_str());
            }
        }
    }

    std::string NodeEditorLayer::FormatParameterName(const std::string &paramName)
    {
        if (paramName.empty())
            return paramName;

        std::string result;
        result.reserve(paramName.length() + 10);
        result += std::toupper(paramName[0]);

        for (size_t i = 1; i < paramName.length(); ++i)
        {
            char current = paramName[i];
            char previous = paramName[i - 1];
            if (std::isupper(current))
            {
                result += ' ';
                result += current;
            }
            else if (std::isdigit(current) && std::isalpha(previous))
            {
                result += ' ';
                result += current;
            }
            else if (std::isalpha(current) && std::isdigit(previous))
            {
                result += ' ';
                result += std::toupper(current);
            }
            else
            {
                result += current;
            }
        }

        return result;
    }

    void NodeEditorLayer::RenderNodeParameters(Engine::Node *node, const ImVec2 &startPos, const ImVec2 &nodeSize)
    {
        auto pins = GetNodePins(node->GetName());
        std::vector<NodePin> parameterPins;
        for (const auto &pin : pins)
        {
            if (pin.isInput && pin.dataType != PinDataType::Image)
            {
                parameterPins.push_back(pin);
            }
        }

        if (parameterPins.empty())
            return;

        ImDrawList *drawList = ImGui::GetWindowDrawList();
        float paramHeight = Constants::Parameter::kHeight * zoomLevel;
        float pinSpacing = Constants::Pin::kSpacing * zoomLevel;
        float padding = Constants::Node::kPadding * zoomLevel;
        float pinRadius = Constants::Pin::kRadius * zoomLevel;

        for (size_t i = 0; i < parameterPins.size(); ++i)
        {
            const auto &pin = parameterPins[i];
            float currentY = startPos.y + i * (paramHeight + pinSpacing);
            ImVec2 pinPos = ImVec2(startPos.x + padding, currentY + pinSpacing * 0.5f);
            ImVec2 labelPos = ImVec2(pinPos.x + pinRadius + Constants::Pin::kTextOffset * zoomLevel, currentY - Constants::Parameter::kLabelOffset * zoomLevel);
            ImVec2 inputPos = ImVec2(startPos.x + padding, currentY + Constants::Parameter::kInputOffset * zoomLevel);

            RenderPin(pin, pinPos, pinRadius);

            std::string displayName = FormatParameterName(pin.name);
            drawList->AddText(labelPos, Constants::Colors::Pin::kLabel, displayName.c_str());

            ImGui::SetCursorScreenPos(inputPos);

            auto currentValue = node->GetParamValue(pin.name);
            std::string paramValue = currentValue.has_value() ? currentValue.value() : "";
            std::string widgetId = "##" + std::to_string(node->GetId()) + "_" + pin.name;

            float availableWidth = nodeSize.x - padding * 2;
            float inputWidth = std::max(Constants::Parameter::kMinInputWidth * zoomLevel, availableWidth);

            switch (pin.dataType)
            {
            case PinDataType::String: {
                char buffer[Constants::Special::kStringBufferSize];
                strncpy_s(buffer, paramValue.c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';

                ImGui::PushItemWidth(inputWidth);
                if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
                {
                    node->SetParamValue(pin.name, std::string(buffer));
                }
                ImGui::PopItemWidth();
                break;
            }

            case PinDataType::Float: {
                float value = 0.0f;
                if (!paramValue.empty())
                {
                    try
                    {
                        value = std::stof(paramValue);
                    }
                    catch (...)
                    {
                        value = 0.0f;
                    }
                }

                ImGui::PushItemWidth(inputWidth);
                if (ImGui::InputFloat(widgetId.c_str(), &value, Constants::Input::Float::kStep, Constants::Input::Float::kFastStep, Constants::Input::Float::kFormat))
                {
                    node->SetParamValue(pin.name, std::to_string(value));
                }
                ImGui::PopItemWidth();
                break;
            }

            case PinDataType::Int: {
                int value = 0;
                if (!paramValue.empty())
                {
                    try
                    {
                        value = std::stoi(paramValue);
                    }
                    catch (...)
                    {
                        value = 0;
                    }
                }

                ImGui::PushItemWidth(inputWidth);
                if (ImGui::InputInt(widgetId.c_str(), &value))
                {
                    node->SetParamValue(pin.name, std::to_string(value));
                }
                ImGui::PopItemWidth();
                break;
            }

            case PinDataType::Bool: {
                bool value = false;
                if (!paramValue.empty())
                {
                    value = (paramValue == "true" || paramValue == "1" || paramValue == "yes" || paramValue == "on");
                }

                if (ImGui::Checkbox((displayName + widgetId).c_str(), &value))
                {
                    node->SetParamValue(pin.name, value ? "true" : "false");
                }
                break;
            }

            default:
                ImGui::Text("%s", paramValue.c_str());
                break;
            }
        }
    }

    std::vector<NodePin> NodeEditorLayer::GetNodeParameters(const std::string &nodeName) const
    {
        const auto allPins = GetNodePins(nodeName);
        std::vector<NodePin> parameterPins;

        for (const auto &pin : allPins)
        {
            if (pin.isInput && pin.dataType != PinDataType::Image)
            {
                parameterPins.push_back(pin);
            }
        }

        return parameterPins;
    }

    Engine::NodeId NodeEditorLayer::FindNodeAtPosition(const ImVec2 &mousePos) const
    {
        const auto nodeIds = nodeEditor.GetNodeIds();

        // Iterate in reverse order to find topmost node
        for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
        {
            const auto nodeId = *it;
            const auto *node = nodeEditor.GetNode(nodeId);

            if (!node || nodePositions.find(nodeId) == nodePositions.end())
                continue;

            const auto pins = GetNodePins(node->GetName());
            const auto dimensions = CalculateNodeDimensions(pins, zoomLevel);

            if (IsMouseOverNode(mousePos, nodePositions.at(nodeId), dimensions.size))
            {
                return nodeId;
            }
        }

        return Constants::Special::kInvalidNodeId;
    }

    PinId NodeEditorLayer::FindPinAtPosition(const ImVec2 &mousePos) const
    {
        const auto nodeIds = nodeEditor.GetNodeIds();

        // Check all nodes for pin hits
        for (const auto nodeId : nodeIds)
        {
            const auto* node = nodeEditor.GetNode(nodeId);
            if (!node || nodePositions.find(nodeId) == nodePositions.end())
                continue;

            const auto pins = GetNodePins(node->GetName());
            const auto dimensions = CalculateNodeDimensions(pins, zoomLevel);
            const auto& nodePos = nodePositions.at(nodeId);

            const auto nodeWorldPos = ImVec2(currentCanvasPos.x + (nodePos.x * zoomLevel + panX),
                                           currentCanvasPos.y + (nodePos.y * zoomLevel + panY));

            // Check input pins (Image pins only for connections)
            std::vector<NodePin> inputPins;
            for (const auto& pin : pins)
            {
                if (pin.isInput && pin.dataType == PinDataType::Image)
                {
                    inputPins.push_back(pin);
                }
            }

            const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
            const auto pinHeight = Constants::Pin::kHeight * zoomLevel;
            const auto pinSpacing = Constants::Pin::kSpacing * zoomLevel;
            const auto padding = Constants::Node::kPadding * zoomLevel;
            const auto pinRadius = Constants::Pin::kRadius * zoomLevel;

            const auto leftColumnX = nodeWorldPos.x + padding;
            const auto inputY = nodeWorldPos.y + titleHeight + padding;

            for (size_t i = 0; i < inputPins.size(); ++i)
            {
                const auto& pin = inputPins[i];
                const auto pinPos = ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);

                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;

                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }

            // Check output pins
            std::vector<NodePin> outputPins;
            for (const auto& pin : pins)
            {
                if (!pin.isInput)
                {
                    outputPins.push_back(pin);
                }
            }

            const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
            const auto outputY = nodeWorldPos.y + titleHeight + padding;

            for (size_t i = 0; i < outputPins.size(); ++i)
            {
                const auto& pin = outputPins[i];
                const auto pinPos = ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);

                const auto distance = ImVec2(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
                const auto distanceSquared = distance.x * distance.x + distance.y * distance.y;

                if (distanceSquared <= pinRadius * pinRadius)
                {
                    return { nodeId, pin.name };
                }
            }
        }

        return { Constants::Special::kInvalidNodeId, "" }; // No pin found
    }

    ImVec2 NodeEditorLayer::GetPinWorldPosition(const PinId &pinId) const
    {
        if (pinId.nodeId == Constants::Special::kInvalidNodeId || nodePositions.find(pinId.nodeId) == nodePositions.end())
            return ImVec2(0, 0);

        const auto* node = nodeEditor.GetNode(pinId.nodeId);
        if (!node)
            return ImVec2(0, 0);

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = CalculateNodeDimensions(pins, zoomLevel);
        const auto& nodePos = nodePositions.at(pinId.nodeId);

        const auto nodeWorldPos = ImVec2(currentCanvasPos.x + (nodePos.x * zoomLevel + panX),
                                       currentCanvasPos.y + (nodePos.y * zoomLevel + panY));

        // Find the specific pin
        const auto titleHeight = Constants::Node::kTitleHeight * zoomLevel;
        const auto pinHeight = Constants::Pin::kHeight * zoomLevel;
        const auto pinSpacing = Constants::Pin::kSpacing * zoomLevel;
        const auto padding = Constants::Node::kPadding * zoomLevel;

        // Check input pins first
        std::vector<NodePin> inputPins;
        for (const auto& pin : pins)
        {
            if (pin.isInput && pin.dataType == PinDataType::Image)
            {
                inputPins.push_back(pin);
            }
        }

        const auto leftColumnX = nodeWorldPos.x + padding;
        const auto inputY = nodeWorldPos.y + titleHeight + padding;

        for (size_t i = 0; i < inputPins.size(); ++i)
        {
            if (inputPins[i].name == pinId.pinName)
            {
                return ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            }
        }

        // Check output pins
        std::vector<NodePin> outputPins;
        for (const auto& pin : pins)
        {
            if (!pin.isInput)
            {
                outputPins.push_back(pin);
            }
        }

        const auto rightColumnX = nodeWorldPos.x + dimensions.size.x - padding;
        const auto outputY = nodeWorldPos.y + titleHeight + padding;

        for (size_t i = 0; i < outputPins.size(); ++i)
        {
            if (outputPins[i].name == pinId.pinName)
            {
                return ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            }
        }

        return ImVec2(0, 0); // Pin not found
    }

    bool NodeEditorLayer::IsConnectionValid(const PinId &outputPin, const PinId &inputPin) const
    {
        // Can't connect to same node
        if (outputPin.nodeId == inputPin.nodeId)
            return false;

        // Get pin information
        const auto* outputNode = nodeEditor.GetNode(outputPin.nodeId);
        const auto* inputNode = nodeEditor.GetNode(inputPin.nodeId);

        if (!outputNode || !inputNode)
            return false;

        const auto outputPins = GetNodePins(outputNode->GetName());
        const auto inputPins = GetNodePins(inputNode->GetName());

        // Find the actual pins
        NodePin actualOutputPin;
        NodePin actualInputPin;
        bool foundOutput = false, foundInput = false;

        for (const auto& pin : outputPins)
        {
            if (pin.name == outputPin.pinName)
            {
                actualOutputPin = pin;
                foundOutput = true;
                break;
            }
        }

        for (const auto& pin : inputPins)
        {
            if (pin.name == inputPin.pinName)
            {
                actualInputPin = pin;
                foundInput = true;
                break;
            }
        }

        if (!foundOutput || !foundInput)
            return false;

        // Validate pin directions
        if (actualOutputPin.isInput || !actualInputPin.isInput)
            return false;

        // Validate data types match
        if (actualOutputPin.dataType != actualInputPin.dataType)
            return false;

        return true;
    }

    bool NodeEditorLayer::CreateConnection(const PinId &outputPin, const PinId &inputPin)
    {
        if (!IsConnectionValid(outputPin, inputPin))
            return false;

        // Remove any existing connection to the input pin (inputs can only have one connection)
        RemoveConnectionToInput(inputPin);

        // Create new connection
        connections.push_back({ outputPin, inputPin });
        return true;
    }

    void NodeEditorLayer::RemoveConnectionToInput(const PinId &inputPin)
    {
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [&inputPin](const NodeConnection& conn) {
                    return conn.inputPin == inputPin;
                }),
            connections.end()
        );
    }

    void NodeEditorLayer::HandleConnectionInteractions()
    {
        const auto& io = ImGui::GetIO();
        const auto mousePos = io.MousePos;

        // Handle connection creation
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // Don't start connection if interacting with UI elements
            if (io.WantCaptureMouse && ImGui::IsAnyItemActive())
                return;

            const auto clickedPin = FindPinAtPosition(mousePos);

            if (clickedPin.nodeId != Constants::Special::kInvalidNodeId)
            {
                if (!connectionState.isCreating)
                {
                    // Start creating connection
                    connectionState.isCreating = true;
                    connectionState.startPin = clickedPin;
                    connectionState.endPosition = mousePos;
                }
                else
                {
                    // Try to complete connection
                    const auto& startPin = connectionState.startPin;
                    const auto& endPin = clickedPin;

                    // Determine which is output and which is input
                    const auto* startNode = nodeEditor.GetNode(startPin.nodeId);
                    const auto* endNode = nodeEditor.GetNode(endPin.nodeId);

                    if (startNode && endNode)
                    {
                        const auto startPins = GetNodePins(startNode->GetName());
                        const auto endPins = GetNodePins(endNode->GetName());

                        bool startIsOutput = false, endIsInput = false;

                        // Check if start pin is output
                        for (const auto& pin : startPins)
                        {
                            if (pin.name == startPin.pinName && !pin.isInput)
                            {
                                startIsOutput = true;
                                break;
                            }
                        }

                        // Check if end pin is input
                        for (const auto& pin : endPins)
                        {
                            if (pin.name == endPin.pinName && pin.isInput)
                            {
                                endIsInput = true;
                                break;
                            }
                        }

                        // Try to create connection in both directions
                        bool connectionCreated = false;
                        if (startIsOutput && endIsInput)
                        {
                            connectionCreated = CreateConnection(startPin, endPin);
                        }
                        else if (!startIsOutput && !endIsInput)
                        {
                            // Start is input, end is output - swap them
                            connectionCreated = CreateConnection(endPin, startPin);
                        }
                    }

                    // Reset connection state
                    connectionState.isCreating = false;
                }
            }
            else if (connectionState.isCreating)
            {
                // Clicked empty space while creating - cancel connection
                connectionState.isCreating = false;
            }
        }

        // Update connection end position while dragging
        if (connectionState.isCreating)
        {
            connectionState.endPosition = mousePos;
        }

        // Cancel connection on right click or escape
        if (connectionState.isCreating && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            connectionState.isCreating = false;
        }
    }

    void NodeEditorLayer::RenderConnections()
    {
        auto* drawList = ImGui::GetWindowDrawList();

        // Render existing connections
        for (const auto& connection : connections)
        {
            RenderConnection(connection);
        }

        // Render connection being created
        if (connectionState.isCreating)
        {
            const auto startPos = GetPinWorldPosition(connectionState.startPin);
            const auto endPos = connectionState.endPosition;

            // Add constants for connection rendering
            const auto connectionColor = Constants::Colors::Connection::kCreating;
            const auto connectionThickness = Constants::Connection::kThickness;

            // Simple line for now (will be improved to bezier curve)
            drawList->AddLine(startPos, endPos, connectionColor, connectionThickness);
        }
    }

    void NodeEditorLayer::RenderConnection(const NodeConnection &connection)
    {
        auto* drawList = ImGui::GetWindowDrawList();

        const auto startPos = GetPinWorldPosition(connection.outputPin);
        const auto endPos = GetPinWorldPosition(connection.inputPin);

        // Skip if either pin position is invalid
        if (startPos.x == 0 && startPos.y == 0)
            return;
        if (endPos.x == 0 && endPos.y == 0)
            return;

        // Add constants for connection rendering
        const auto connectionColor = Constants::Colors::Connection::kActive;
        const auto connectionThickness = Constants::Connection::kThickness;
        const auto bezierTension = Constants::Connection::kBezierTension;

        // Calculate bezier control points for a nice curve
        const auto distance = std::abs(endPos.x - startPos.x);
        const auto tension = std::min(distance * 0.5f, bezierTension * zoomLevel);

        const auto cp1 = ImVec2(startPos.x + tension, startPos.y);
        const auto cp2 = ImVec2(endPos.x - tension, endPos.y);

        // Render bezier curve
        drawList->AddBezierCubic(startPos, cp1, cp2, endPos, connectionColor, connectionThickness);
    }
} // namespace VisionCraft