#include "NodeEditorLayer.h"

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
    // UI Constants
    constexpr float kDefaultZoomLevel = 1.0f;
    constexpr float kMinZoomLevel = 0.1f;
    constexpr float kMaxZoomLevel = 5.0f;
    constexpr float kZoomStep = 0.1f;
    constexpr float kMinCanvasSize = 50.0f;

    // Node Visual Constants
    constexpr float kNodeWidth = 220.0f;
    constexpr float kMinNodeHeight = 100.0f;
    constexpr float kTitleHeight = 25.0f;
    constexpr float kPinHeight = 18.0f;
    constexpr float kParamHeight = 22.0f;
    constexpr float kPinSpacing = 3.0f;
    constexpr float kNodePadding = 8.0f;
    constexpr float kPinRadius = 5.0f;
    constexpr float kNodeRounding = 8.0f;
    constexpr float kBorderThicknessNormal = 2.0f;
    constexpr float kBorderThicknessSelected = 3.0f;
    constexpr float kPinBorderThickness = 1.5f;
    constexpr float kTextOffset = 4.0f;
    constexpr float kMinZoomForText = 0.5f;

    // Node Creation Offset
    constexpr float kNodeCreationOffsetX = 100.0f;
    constexpr float kNodeCreationOffsetY = 40.0f;

    // Grid Transparency
    constexpr int kGridAlpha = 40;

    // Invalid Node ID
    constexpr VisionCraft::Engine::NodeId kInvalidNodeId = -1;

    // Color Constants
    constexpr ImU32 kNodeColor = IM_COL32(60, 60, 60, 255);
    constexpr ImU32 kBorderColorNormal = IM_COL32(100, 100, 100, 255);
    constexpr ImU32 kBorderColorSelected = IM_COL32(255, 165, 0, 255);
    constexpr ImU32 kTitleColor = IM_COL32(80, 80, 120, 255);
    constexpr ImU32 kTextColor = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 kPinLabelColor = IM_COL32(200, 200, 200, 255);
    constexpr ImU32 kPinBorderColor = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 kGridColor = IM_COL32(200, 200, 200, kGridAlpha);

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

        const auto titleHeight = kTitleHeight * zoomLevel;
        const auto pinHeight = kPinHeight * zoomLevel;
        const auto paramHeight = kParamHeight * zoomLevel;
        const auto pinSpacing = kPinSpacing * zoomLevel;
        const auto padding = kNodePadding * zoomLevel;

        const auto maxPins = std::max(inputPins.size(), outputPins.size());
        const auto pinsHeight = maxPins * (pinHeight + pinSpacing);
        const auto parametersHeight = parameterPins.size() * (paramHeight + pinSpacing);
        const auto contentHeight = pinsHeight + parametersHeight + padding * 2;
        const auto totalHeight = titleHeight + contentHeight + padding;

        return { ImVec2(kNodeWidth * zoomLevel, std::max(kMinNodeHeight * zoomLevel, totalHeight)),
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

        if (canvas_size.x < kMinCanvasSize)
        {
            canvas_size.x = kMinCanvasSize;
        }

        if (canvas_size.y < kMinCanvasSize)
        {
            canvas_size.y = kMinCanvasSize;
        }

        if (showGrid)
        {
            const auto grid_step = gridSize * zoomLevel;

            for (float x = fmodf(panX, grid_step); x < canvas_size.x; x += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y),
                    ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
                    kGridColor);
            }

            for (float y = fmodf(panY, grid_step); y < canvas_size.y; y += grid_step)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y),
                    ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
                    kGridColor);
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
                zoomLevel += io.MouseWheel * kZoomStep;
                zoomLevel = std::clamp(zoomLevel, kMinZoomLevel, kMaxZoomLevel);
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
        auto *drawList = ImGui::GetWindowDrawList();
        const auto canvasPos = currentCanvasPos;

        const auto worldPos =
            ImVec2(canvasPos.x + (nodePos.x * zoomLevel + panX), canvasPos.y + (nodePos.y * zoomLevel + panY));

        const auto pins = GetNodePins(node->GetName());
        const auto dimensions = CalculateNodeDimensions(pins, zoomLevel);

        const auto isSelected = (node->GetId() == selectedNodeId);
        const auto borderColor = isSelected ? kBorderColorSelected : kBorderColorNormal;
        const auto borderThickness = isSelected ? kBorderThicknessSelected : kBorderThicknessNormal;
        const auto nodeRounding = kNodeRounding * zoomLevel;

        // Draw node background
        drawList->AddRectFilled(
            worldPos, ImVec2(worldPos.x + dimensions.size.x, worldPos.y + dimensions.size.y), kNodeColor, nodeRounding);

        // Draw node border
        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + dimensions.size.x, worldPos.y + dimensions.size.y),
            borderColor,
            nodeRounding,
            0,
            borderThickness * zoomLevel);

        // Draw title background
        const auto titleHeight = kTitleHeight * zoomLevel;
        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + dimensions.size.x, worldPos.y + titleHeight),
            kTitleColor,
            nodeRounding,
            ImDrawFlags_RoundCornersTop);

        // Draw title text
        if (zoomLevel > kMinZoomForText)
        {
            const auto textPos = ImVec2(worldPos.x + kNodePadding * zoomLevel, worldPos.y + kTextOffset * zoomLevel);
            drawList->AddText(textPos, kTextColor, node->GetName().c_str());
        }

        // Render pins
        RenderNodePins(pins, worldPos, dimensions, zoomLevel);

        // Render parameters if needed
        if (dimensions.parameterPinCount > 0 && zoomLevel > kMinZoomForText)
        {
            const auto parametersStartY = worldPos.y + titleHeight + (kNodePadding * zoomLevel)
                                          + (std::max(dimensions.inputPinCount, dimensions.outputPinCount)
                                              * (kPinHeight + kPinSpacing) * zoomLevel)
                                          + (kNodePadding * zoomLevel);
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
            if (clickedNodeId == kInvalidNodeId)
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

            if (clickedNodeId != kInvalidNodeId)
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
                selectedNodeId = kInvalidNodeId;
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
        auto *drawList = ImGui::GetWindowDrawList();
        const auto pinColor = GetDataTypeColor(pin.dataType);

        drawList->AddCircleFilled(position, radius, pinColor);
        drawList->AddCircle(position, radius, kPinBorderColor, 0, kPinBorderThickness);
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

        const auto titleHeight = kTitleHeight * zoomLevel;
        const auto pinHeight = kPinHeight * zoomLevel;
        const auto pinSpacing = kPinSpacing * zoomLevel;
        const auto padding = kNodePadding * zoomLevel;
        const auto pinRadius = kPinRadius * zoomLevel;
        const auto textOffset = kTextOffset * zoomLevel;

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

            if (zoomLevel > kMinZoomForText)
            {
                const auto displayName = FormatParameterName(pin.name);
                drawList->AddText(labelPos, kPinLabelColor, displayName.c_str());
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

            if (zoomLevel > kMinZoomForText)
            {
                drawList->AddText(labelPos, kPinLabelColor, displayName.c_str());
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

        return kInvalidNodeId;
    }
} // namespace VisionCraft