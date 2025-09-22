#include "NodeEditorLayer.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/ThresholdNode.h"

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

        if (canvas_size.x < 50.0f)
        {
            canvas_size.x = 50.0f;
        }

        if (canvas_size.y < 50.0f)
        {
            canvas_size.y = 50.0f;
        }

        if (showGrid)
        {
            ImU32 grid_color = IM_COL32(200, 200, 200, 40);
            float grid_step = gridSize * zoomLevel;

            for (float x = fmodf(panX, grid_step); x < canvas_size.x; x += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y),
                    ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
                    grid_color);
            }

            for (float y = fmodf(panY, grid_step); y < canvas_size.y; y += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y),
                    ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
                    grid_color);
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
                zoomLevel += io.MouseWheel * 0.1f;
                zoomLevel = std::clamp(zoomLevel, 0.1f, 5.0f);
            }
        }

        if (nodeEditor.GetNodeIds().empty())
        {
            auto starterNode = std::make_unique<Engine::ImageInputNode>(nextNodeId++);
            Engine::NodeId nodeId = starterNode->GetId();
            nodeEditor.AddNode(std::move(starterNode));
            nodePositions[nodeId] = { 100.0f, 100.0f };

            selectedNodeId = -1;
            isDragging = false;
            dragOffset = ImVec2(0.0f, 0.0f);
        }

        HandleMouseInteractions();

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
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = currentCanvasPos;

        ImVec2 worldPos =
            ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));

        auto pins = GetNodePins(node->GetName());

        std::vector<NodePin> inputPins;
        std::vector<NodePin> outputPins;
        std::vector<NodePin> parameterPins;
        for (const auto &pin : pins)
        {
            if (pin.isInput)
            {
                if (pin.dataType == PinDataType::Image)
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

        float titleHeight = 25.0f * zoomLevel;
        float pinHeight = 18.0f * zoomLevel;
        float paramHeight = 22.0f * zoomLevel;
        float pinSpacing = 3.0f * zoomLevel;
        float padding = 8.0f * zoomLevel;

        size_t maxPins = std::max(inputPins.size(), outputPins.size());
        float pinsHeight = maxPins * (pinHeight + pinSpacing);
        float parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
        float contentHeight = pinsHeight + parametersHeight + padding * 2;
        float totalHeight = titleHeight + contentHeight + padding;

        ImVec2 nodeSize = ImVec2(220.0f * zoomLevel, std::max(100.0f * zoomLevel, totalHeight));

        bool isSelected = (node->GetId() == selectedNodeId);

        ImU32 nodeColor = IM_COL32(60, 60, 60, 255);
        ImU32 borderColor = isSelected ? IM_COL32(255, 165, 0, 255) : IM_COL32(100, 100, 100, 255);
        ImU32 titleColor = IM_COL32(80, 80, 120, 255);

        drawList->AddRectFilled(
            worldPos, ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y), nodeColor, 8.0f * zoomLevel);

        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y),
            borderColor,
            8.0f * zoomLevel,
            0,
            isSelected ? 3.0f * zoomLevel : 2.0f * zoomLevel);

        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + titleHeight),
            titleColor,
            8.0f * zoomLevel,
            ImDrawFlags_RoundCornersTop);

        if (zoomLevel > 0.5f)
        {
            ImVec2 textPos = ImVec2(worldPos.x + 8.0f * zoomLevel, worldPos.y + 4.0f * zoomLevel);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), node->GetName().c_str());
        }

        float pinRadius = 5.0f * zoomLevel;
        float leftColumnX = worldPos.x + 8.0f * zoomLevel;
        float rightColumnX = worldPos.x + nodeSize.x - 8.0f * zoomLevel;

        float inputY = worldPos.y + titleHeight + padding;
        for (size_t i = 0; i < inputPins.size(); ++i)
        {
            const auto &pin = inputPins[i];

            ImVec2 pinPos = ImVec2(leftColumnX, inputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            ImVec2 labelPos = ImVec2(leftColumnX + pinRadius + 4.0f * zoomLevel, inputY + i * (pinHeight + pinSpacing));

            RenderPin(pin, pinPos, pinRadius);

            if (zoomLevel > 0.5f)
            {
                std::string displayName = FormatParameterName(pin.name);
                drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), displayName.c_str());
            }
        }

        float outputY = worldPos.y + titleHeight + padding;
        for (size_t i = 0; i < outputPins.size(); ++i)
        {
            const auto &pin = outputPins[i];

            std::string displayName = FormatParameterName(pin.name);
            ImVec2 textSize = ImGui::CalcTextSize(displayName.c_str());
            ImVec2 pinPos = ImVec2(rightColumnX, outputY + i * (pinHeight + pinSpacing) + pinHeight * 0.5f);
            ImVec2 labelPos = ImVec2(
                rightColumnX - pinRadius - textSize.x - 4.0f * zoomLevel, outputY + i * (pinHeight + pinSpacing));

            RenderPin(pin, pinPos, pinRadius);

            if (zoomLevel > 0.5f)
            {
                drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), displayName.c_str());
            }
        }

        if (!parameterPins.empty() && zoomLevel > 0.5f)
        {
            float parametersStartY = worldPos.y + titleHeight + padding + pinsHeight + padding;
            RenderNodeParameters(node, ImVec2(worldPos.x, parametersStartY), nodeSize);
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
            Engine::NodeId clickedNodeId = -1;

            auto nodeIds = nodeEditor.GetNodeIds();
            for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
            {
                Engine::NodeId nodeId = *it;
                Engine::Node *node = nodeEditor.GetNode(nodeId);
                if (node && nodePositions.find(nodeId) != nodePositions.end())
                {
                    auto pins = GetNodePins(node->GetName());
                    std::vector<NodePin> inputPins, outputPins, parameterPins;
                    for (const auto &pin : pins)
                    {
                        if (pin.isInput)
                        {
                            if (pin.dataType == PinDataType::Image)
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

                    float titleHeight = 25.0f * zoomLevel;
                    float pinHeight = 18.0f * zoomLevel;
                    float paramHeight = 22.0f * zoomLevel;
                    float pinSpacing = 3.0f * zoomLevel;
                    float padding = 8.0f * zoomLevel;

                    size_t maxPins = std::max(inputPins.size(), outputPins.size());
                    float pinsHeight = maxPins * (pinHeight + pinSpacing);
                    float parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
                    float contentHeight = pinsHeight + parametersHeight + padding * 2;
                    float totalHeight = titleHeight + contentHeight + padding;

                    ImVec2 nodeSize = ImVec2(220.0f * zoomLevel, std::max(100.0f * zoomLevel, totalHeight));

                    if (IsMouseOverNode(mousePos, nodePositions[nodeId], nodeSize))
                    {
                        clickedNodeId = nodeId;
                        break;
                    }
                }
            }

            if (clickedNodeId == -1)
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

            Engine::NodeId clickedNodeId = -1;

            auto nodeIds = nodeEditor.GetNodeIds();
            for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it)
            {
                Engine::NodeId nodeId = *it;
                Engine::Node *node = nodeEditor.GetNode(nodeId);
                if (node && nodePositions.find(nodeId) != nodePositions.end())
                {
                    auto pins = GetNodePins(node->GetName());
                    std::vector<NodePin> inputPins, outputPins, parameterPins;
                    for (const auto &pin : pins)
                    {
                        if (pin.isInput)
                        {
                            if (pin.dataType == PinDataType::Image)
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

                    float titleHeight = 25.0f * zoomLevel;
                    float pinHeight = 18.0f * zoomLevel;
                    float paramHeight = 22.0f * zoomLevel;
                    float pinSpacing = 3.0f * zoomLevel;
                    float padding = 8.0f * zoomLevel;

                    size_t maxPins = std::max(inputPins.size(), outputPins.size());
                    float pinsHeight = maxPins * (pinHeight + pinSpacing);
                    float parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
                    float contentHeight = pinsHeight + parametersHeight + padding * 2;
                    float totalHeight = titleHeight + contentHeight + padding;

                    ImVec2 nodeSize = ImVec2(220.0f * zoomLevel, std::max(100.0f * zoomLevel, totalHeight));

                    if (IsMouseOverNode(mousePos, nodePositions[nodeId], nodeSize))
                    {
                        clickedNodeId = nodeId;
                        break;
                    }
                }
            }

            if (clickedNodeId != -1)
            {
                selectedNodeId = clickedNodeId;
                isDragging = true;

                const NodePosition &nodePos = nodePositions[clickedNodeId];
                ImVec2 worldPos =
                    ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));
                dragOffset = ImVec2(mousePos.x - worldPos.x, mousePos.y - worldPos.y);
            }
            else
            {
                selectedNodeId = -1;
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

        worldX -= 100.0f;
        worldY -= 40.0f;

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

            selectedNodeId = -1;
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
            return IM_COL32(100, 200, 100, 255);
        case PinDataType::String:
            return IM_COL32(200, 100, 200, 255);
        case PinDataType::Float:
            return IM_COL32(100, 150, 255, 255);
        case PinDataType::Int:
            return IM_COL32(100, 255, 255, 255);
        case PinDataType::Bool:
            return IM_COL32(255, 100, 100, 255);
        default:
            return IM_COL32(128, 128, 128, 255);
        }
    }

    void NodeEditorLayer::RenderPin(const NodePin &pin, const ImVec2 &position, float radius) const
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImU32 pinColor = GetDataTypeColor(pin.dataType);

        drawList->AddCircleFilled(position, radius, pinColor);
        drawList->AddCircle(position, radius, IM_COL32(255, 255, 255, 255), 0, 1.5f);
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
        float paramHeight = 22.0f * zoomLevel;
        float pinSpacing = 3.0f * zoomLevel;
        float padding = 8.0f * zoomLevel;
        float pinRadius = 5.0f * zoomLevel;

        for (size_t i = 0; i < parameterPins.size(); ++i)
        {
            const auto &pin = parameterPins[i];
            float currentY = startPos.y + i * (paramHeight + pinSpacing);
            ImVec2 pinPos = ImVec2(startPos.x + padding, currentY + pinSpacing * 0.5f);
            ImVec2 labelPos = ImVec2(pinPos.x + pinRadius + 4.0f * zoomLevel, currentY - 2.0f * zoomLevel);
            ImVec2 inputPos = ImVec2(startPos.x + padding, currentY + 12.0f * zoomLevel);

            RenderPin(pin, pinPos, pinRadius);

            std::string displayName = FormatParameterName(pin.name);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), displayName.c_str());

            ImGui::SetCursorScreenPos(inputPos);

            auto currentValue = node->GetParamValue(pin.name);
            std::string paramValue = currentValue.has_value() ? currentValue.value() : "";
            std::string widgetId = "##" + std::to_string(node->GetId()) + "_" + pin.name;

            float availableWidth = nodeSize.x - padding * 2;
            float inputWidth = std::max(80.0f * zoomLevel, availableWidth);

            switch (pin.dataType)
            {
            case PinDataType::String: {
                char buffer[256];
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
                if (ImGui::InputFloat(widgetId.c_str(), &value, 0.1f, 1.0f, "%.2f"))
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
        auto allPins = GetNodePins(nodeName);
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
} // namespace VisionCraft