#include "NodeRenderer.h"
#include "DefaultNodeRenderingStrategy.h"
#include "ImageInputNodeRenderingStrategy.h"
#include "NodeDimensionCalculator.h"
#include "NodeEditorConstants.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/PreviewNode.h"
#include "PreviewNodeRenderingStrategy.h"

#include <ImGuiFileDialog.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iterator>

namespace VisionCraft
{
    NodeRenderer::NodeRenderer(CanvasController &canvas, ConnectionManager &connectionManager)
        : canvas_(canvas), connectionManager_(connectionManager)
    {
    }

    void NodeRenderer::RenderNode(Engine::Node *node,
        const NodePosition &nodePos,
        Engine::NodeId selectedNodeId,
        std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState)
    {
        const auto worldPos = canvas_.WorldToScreen(ImVec2(nodePos.x, nodePos.y));
        const auto pins = connectionManager_.GetNodePins(node->GetName());
        const auto dimensions = NodeRenderer::CalculateNodeDimensions(pins, canvas_.GetZoomLevel(), node);
        const auto isSelected = (node->GetId() == selectedNodeId);

        RenderNodeBackground(worldPos, dimensions.size, isSelected);
        RenderNodeTitleBar(worldPos, dimensions.size);
        RenderNodeTitleText(node, worldPos);

        auto [inputPins, outputPins] = SeparateInputOutputPins(pins);
        RenderPinsInColumn(node, inputPins, worldPos, dimensions, true, getPinInteractionState);
        RenderPinsInColumn(node, outputPins, worldPos, dimensions, false, getPinInteractionState);

        RenderCustomNodeContent(node, worldPos, dimensions.size);
        RenderFileBrowser();
    }

    void NodeRenderer::RenderNodeBackground(const ImVec2 &worldPos, const ImVec2 &nodeSize, bool isSelected)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        const auto borderColor =
            isSelected ? Constants::Colors::Node::kBorderSelected : Constants::Colors::Node::kBorderNormal;
        const auto borderThickness =
            isSelected ? Constants::Node::Border::kThicknessSelected : Constants::Node::Border::kThicknessNormal;
        const auto nodeRounding = Constants::Node::kRounding * canvas_.GetZoomLevel();

        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y),
            Constants::Colors::Node::kBackground,
            nodeRounding);

        drawList->AddRect(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + nodeSize.y),
            borderColor,
            nodeRounding,
            0,
            borderThickness * canvas_.GetZoomLevel());
    }

    void NodeRenderer::RenderNodeTitleBar(const ImVec2 &worldPos, const ImVec2 &nodeSize)
    {
        auto *drawList = ImGui::GetWindowDrawList();
        const auto titleHeight = Constants::Node::kTitleHeight * canvas_.GetZoomLevel();
        const auto nodeRounding = Constants::Node::kRounding * canvas_.GetZoomLevel();

        drawList->AddRectFilled(worldPos,
            ImVec2(worldPos.x + nodeSize.x, worldPos.y + titleHeight),
            Constants::Colors::Node::kTitle,
            nodeRounding,
            ImDrawFlags_RoundCornersTop);
    }

    void NodeRenderer::RenderNodeTitleText(Engine::Node *node, const ImVec2 &worldPos)
    {
        if (canvas_.GetZoomLevel() > Constants::Zoom::kMinForText)
        {
            auto *drawList = ImGui::GetWindowDrawList();
            const auto textPos = ImVec2(worldPos.x + Constants::Node::kPadding * canvas_.GetZoomLevel(),
                worldPos.y + Constants::Node::Text::kOffset * canvas_.GetZoomLevel());
            drawList->AddText(textPos, Constants::Colors::Node::kText, node->GetName().c_str());
        }
    }

    std::pair<std::vector<NodePin>, std::vector<NodePin>> NodeRenderer::SeparateInputOutputPins(
        const std::vector<NodePin> &pins)
    {
        std::vector<NodePin> inputPins;
        std::vector<NodePin> outputPins;

        for (const auto &pin : pins)
        {
            if (pin.isInput)
            {
                inputPins.push_back(pin);
            }
            else
            {
                outputPins.push_back(pin);
            }
        }

        return { inputPins, outputPins };
    }

    void NodeRenderer::RenderNodeParametersInColumns(Engine::Node *node,
        const ImVec2 &startPos,
        const ImVec2 &nodeSize,
        const std::vector<NodePin> &inputPins,
        const std::vector<NodePin> &outputPins)
    {
        if (inputPins.empty() && outputPins.empty())
        {
            return;
        }

        const auto layout = CalculateColumnLayout(nodeSize, inputPins.size(), outputPins.size());
        auto *drawList = ImGui::GetWindowDrawList();
        const auto paramHeight = Constants::Parameter::kHeight * canvas_.GetZoomLevel();
        const auto paramSpacing = Constants::Parameter::kSpacing * canvas_.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas_.GetZoomLevel();
        for (size_t i = 0; i < inputPins.size(); ++i)
        {
            const auto &pin = inputPins[i];
            const auto rowY = startPos.y + (i % static_cast<size_t>(layout.maxRows)) * layout.rowHeight;
            const auto columnIndex = i / static_cast<size_t>(layout.maxRows);
            const auto columnX = startPos.x + layout.leftColumnX + (columnIndex * (layout.columnWidth + padding));
            const auto paramStartX = columnX;
            const auto paramEndX = std::min(paramStartX + layout.columnWidth, startPos.x + nodeSize.x - padding);
            const auto paramWidth = paramEndX - paramStartX;
            if (paramWidth > 0)
            {
                RenderParameterInColumn(node, pin, ImVec2(paramStartX, rowY), paramWidth);
            }
        }

        for (size_t i = 0; i < outputPins.size(); ++i)
        {
            const auto &pin = outputPins[i];
            const auto rowY = startPos.y + (i % static_cast<size_t>(layout.maxRows)) * layout.rowHeight;
            const auto columnIndex = i / static_cast<size_t>(layout.maxRows);
            const auto columnX = startPos.x + layout.rightColumnX - (columnIndex * (layout.columnWidth + padding));
            const auto paramStartX = std::max(columnX - layout.columnWidth, startPos.x + padding);
            const auto paramEndX = columnX;
            const auto paramWidth = paramEndX - paramStartX;
            if (paramWidth > 0)
            {
                RenderParameterInColumn(node, pin, ImVec2(paramStartX, rowY), paramWidth);
            }
        }
    }

    void NodeRenderer::RenderPinsInColumn(Engine::Node *node,
        const std::vector<NodePin> &pins,
        const ImVec2 &nodeWorldPos,
        const NodeDimensions &dimensions,
        bool isInputColumn,
        std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState)
    {
        if (pins.empty())
        {
            return;
        }

        const auto titleHeight = Constants::Node::kTitleHeight * canvas_.GetZoomLevel();
        const auto compactPinHeight = Constants::Pin::kCompactHeight * canvas_.GetZoomLevel();
        const auto extendedPinHeight = Constants::Pin::kHeight * canvas_.GetZoomLevel();
        const auto compactSpacing = Constants::Pin::kCompactSpacing * canvas_.GetZoomLevel();
        const auto normalSpacing = Constants::Pin::kSpacing * canvas_.GetZoomLevel();
        const auto padding = Constants::Node::kPadding * canvas_.GetZoomLevel();
        const auto pinRadius = Constants::Pin::kRadius * canvas_.GetZoomLevel();
        const auto textOffset = Constants::Pin::kTextOffset * canvas_.GetZoomLevel();
        const auto columnX = isInputColumn ? nodeWorldPos.x + padding : nodeWorldPos.x + dimensions.size.x - padding;
        const auto startY = nodeWorldPos.y + titleHeight + padding;
        float currentY = startY;
        for (size_t i = 0; i < pins.size(); ++i)
        {
            const auto &pin = pins[i];
            const bool needsInputWidget = connectionManager_.PinNeedsInputWidget(node->GetId(), pin);
            const auto currentPinHeight = needsInputWidget ? extendedPinHeight : compactPinHeight;
            const auto currentSpacing = needsInputWidget ? normalSpacing : compactSpacing;

            const auto pinPos =
                ImVec2(columnX, currentY + currentPinHeight * Constants::NodeRenderer::PinLayout::kCenterFactor);

            ImVec2 labelPos;
            if (isInputColumn)
            {
                labelPos = ImVec2(columnX + pinRadius + textOffset,
                    currentY + currentPinHeight * Constants::NodeRenderer::PinLayout::kCenterFactor
                        - Constants::NodeRenderer::PinLayout::kTextVerticalOffset * canvas_.GetZoomLevel());
            }
            else
            {
                const auto displayName = FormatParameterName(pin.name);
                const auto textSize = ImGui::CalcTextSize(displayName.c_str());
                labelPos = ImVec2(columnX - pinRadius - textSize.x - textOffset,
                    currentY + currentPinHeight * Constants::NodeRenderer::PinLayout::kCenterFactor
                        - Constants::NodeRenderer::PinLayout::kTextVerticalOffset * canvas_.GetZoomLevel());
            }

            const auto state = getPinInteractionState(node->GetId(), pin.name);
            RenderPinWithLabel(pin, pinPos, labelPos, pinRadius, canvas_.GetZoomLevel(), state);

            if (needsInputWidget)
            {
                RenderParameterInputWidget(node, pin, pinPos, pinRadius);
            }

            currentY += currentPinHeight + currentSpacing;
        }
    }

    void NodeRenderer::RenderPinWithLabel(const NodePin &pin,
        const ImVec2 &pinPos,
        const ImVec2 &labelPos,
        float pinRadius,
        float zoomLevel,
        const PinInteractionState &state) const
    {
        RenderPin(pin, pinPos, pinRadius, state);

        if (zoomLevel > Constants::Zoom::kMinForText)
        {
            const auto displayName = FormatParameterName(pin.name);
            auto *drawList = ImGui::GetWindowDrawList();
            drawList->AddText(labelPos, Constants::Colors::Pin::kLabel, displayName.c_str());
        }
    }

    void NodeRenderer::RenderPin(const NodePin &pin,
        const ImVec2 &position,
        float radius,
        const PinInteractionState &state) const
    {
        auto *drawList = ImGui::GetWindowDrawList();
        auto pinColor = GetDataTypeColor(pin.dataType);
        auto borderColor = Constants::Colors::Pin::kBorder;
        if (state.isActive)
        {
            borderColor = Constants::Colors::Pin::kActive;
            drawList->AddCircleFilled(position,
                radius + Constants::NodeRenderer::PinEffects::kActiveRadiusExpansion,
                Constants::Colors::Pin::kActive);
        }
        else if (state.isHovered)
        {
            borderColor = Constants::Colors::Pin::kHover;
            drawList->AddCircleFilled(position,
                radius + Constants::NodeRenderer::PinEffects::kHoverRadiusExpansion,
                Constants::Colors::Pin::kHover);
        }

        drawList->AddCircleFilled(position, radius, pinColor);
        drawList->AddCircle(position, radius, borderColor, 0, Constants::Pin::kBorderThickness);
    }

    ImU32 NodeRenderer::GetDataTypeColor(PinDataType dataType) const
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

    std::string NodeRenderer::FormatParameterName(const std::string &paramName)
    {
        if (paramName.empty())
        {
            return paramName;
        }

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

    NodeRenderer::ColumnLayout
        NodeRenderer::CalculateColumnLayout(const ImVec2 &nodeSize, size_t inputPinCount, size_t outputPinCount) const
    {
        ColumnLayout layout;

        const auto padding = Constants::Node::kPadding * canvas_.GetZoomLevel();
        const auto totalWidth = nodeSize.x - (2 * padding);

        layout.columnWidth = totalWidth * Constants::NodeRenderer::ColumnLayout::kColumnWidthFactor;
        layout.leftColumnX = padding;
        layout.rightColumnX = nodeSize.x - padding - layout.columnWidth;
        layout.rowHeight = (Constants::Parameter::kHeight + Constants::Parameter::kSpacing) * canvas_.GetZoomLevel();
        layout.maxRows =
            std::max(Constants::NodeRenderer::ColumnLayout::kMinRows, std::floor(nodeSize.y / layout.rowHeight));

        return layout;
    }

    std::string NodeRenderer::FitTextInColumn(const std::string &text, float maxWidth) const
    {
        if (maxWidth <= 0)
        {
            return "";
        }

        const auto fullTextSize = ImGui::CalcTextSize(text.c_str());
        if (fullTextSize.x <= maxWidth)
        {
            return text;
        }

        const auto ellipsis = "...";
        const auto ellipsisSize = ImGui::CalcTextSize(ellipsis);
        const auto availableWidth = maxWidth - ellipsisSize.x;

        if (availableWidth <= 0)
        {
            return ellipsis;
        }

        size_t left = 0;
        size_t right = text.length();
        size_t bestFit = 0;

        while (left <= right)
        {
            const auto mid = (left + right) / 2;
            const auto substring = text.substr(0, mid);
            const auto substringSize = ImGui::CalcTextSize(substring.c_str());

            if (substringSize.x <= availableWidth)
            {
                bestFit = mid;
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        return text.substr(0, bestFit) + ellipsis;
    }


    void NodeRenderer::RenderParameterInColumn(Engine::Node *node,
        const NodePin &pin,
        const ImVec2 &position,
        float columnWidth)
    {
        const auto padding = Constants::Node::kPadding * canvas_.GetZoomLevel();

        const auto labelText = FitTextInColumn(FormatParameterName(pin.name), columnWidth - padding);
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddText(position, Constants::Colors::Pin::kLabel, labelText.c_str());

        const auto inputY = position.y + (Constants::Parameter::kInputOffset * canvas_.GetZoomLevel());
        const auto inputWidth =
            std::max(Constants::Parameter::kMinInputWidth * canvas_.GetZoomLevel(), columnWidth - (2 * padding));

        ImGui::SetCursorScreenPos(ImVec2(position.x + padding, inputY));

        const auto widgetId = "##" + std::to_string(node->GetId()) + "_" + pin.name;

        switch (pin.dataType)
        {
        case PinDataType::String:
            RenderStringInput(node, pin, widgetId, inputWidth);
            break;

        case PinDataType::Float:
            RenderFloatInput(node, pin, widgetId, inputWidth);
            break;

        case PinDataType::Int:
            RenderIntInput(node, pin, widgetId, inputWidth);
            break;

        case PinDataType::Bool:
            RenderBoolInput(node, pin, widgetId);
            break;

        case PinDataType::Path:
            RenderPathInput(node, pin, widgetId, inputWidth);
            break;

        default: {
            std::string paramValue = "Unknown";
            ImGui::Text("%s", FitTextInColumn(paramValue, inputWidth).c_str());
            break;
        }
        }
    }

    void NodeRenderer::RenderParameterInputWidget(Engine::Node *node,
        const NodePin &pin,
        const ImVec2 &pinPos,
        float pinRadius)
    {
        const auto textOffset = Constants::Pin::kTextOffset * canvas_.GetZoomLevel();
        const auto inputVerticalOffset =
            Constants::NodeRenderer::ParameterInput::kVerticalOffset * canvas_.GetZoomLevel();
        const auto inputPosX = pinPos.x + pinRadius + textOffset;
        const auto inputPosY = pinPos.y + inputVerticalOffset;
        const auto inputPos = ImVec2(inputPosX, inputPosY);
        const auto inputWidth = Constants::NodeRenderer::ParameterInput::kInputWidth * canvas_.GetZoomLevel();

        ImGui::SetCursorScreenPos(inputPos);

        const auto widgetId = "##" + std::to_string(node->GetId()) + "_" + pin.name;

        switch (pin.dataType)
        {
        case PinDataType::String:
            RenderStringInput(node, pin, widgetId, inputWidth);
            break;
        case PinDataType::Float:
            RenderFloatInput(node, pin, widgetId, inputWidth);
            break;
        case PinDataType::Int:
            RenderIntInput(node, pin, widgetId, inputWidth);
            break;
        case PinDataType::Bool:
            RenderBoolInput(node, pin, widgetId);
            break;
        case PinDataType::Path:
            RenderPathInput(node, pin, widgetId, inputWidth);
            break;
        default:
            break;
        }
    }

    void NodeRenderer::RenderStringInput(Engine::Node *node,
        const NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        std::string paramValue = node->GetParamOr<std::string>(pin.name, "");
        char buffer[256];
        strncpy_s(buffer, paramValue.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
        {
            node->SetParam(pin.name, std::string(buffer));
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderFloatInput(Engine::Node *node,
        const NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        float value = static_cast<float>(
            node->GetParamOr<double>(pin.name, Constants::NodeRenderer::ParameterInput::kDefaultFloatValue));

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputFloat(widgetId.c_str(),
                &value,
                Constants::NodeRenderer::ParameterInput::kFloatStep,
                Constants::NodeRenderer::ParameterInput::kFloatFastStep,
                Constants::NodeRenderer::ParameterInput::kFloatFormat))
        {
            node->SetParam(pin.name, static_cast<double>(value));
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderIntInput(Engine::Node *node,
        const NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        int value = node->GetParamOr<int>(pin.name, 0);

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputInt(widgetId.c_str(), &value))
        {
            node->SetParam(pin.name, value);
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderBoolInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId)
    {
        bool value = node->GetParamOr<bool>(pin.name, false);

        if (ImGui::Checkbox(widgetId.c_str(), &value))
        {
            node->SetParam(pin.name, value);
        }
    }

    void NodeRenderer::RenderPathInput(Engine::Node *node,
        const NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        auto pathValue = node->GetParamOr<std::filesystem::path>(pin.name, std::filesystem::path{});
        std::string pathStr = pathValue.string();
        char buffer[256];
        strncpy_s(buffer, pathStr.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        bool isImageInputFilepath = (pin.name == "filepath" && dynamic_cast<Engine::ImageInputNode *>(node) != nullptr);

        if (isImageInputFilepath)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
            {
                node->SetParam(pin.name, std::filesystem::path(buffer));
            }
            ImGui::PopItemWidth();

            const float buttonWidth = Constants::NodeRenderer::ParameterInput::kButtonWidth * canvas_.GetZoomLevel();
            const float spacing = Constants::NodeRenderer::ParameterInput::kSpacing * canvas_.GetZoomLevel();
            const float buttonHeight = ImGui::GetFrameHeight();

            ImVec2 buttonPos = ImVec2(cursorPos.x + inputWidth + spacing, cursorPos.y);

            ImGui::SetCursorScreenPos(buttonPos);
            const std::string browseId = "...";
            if (ImGui::Button(browseId.c_str(), ImVec2(buttonWidth, buttonHeight)))
            {
                fileBrowserOpen = true;
                fileBrowserTargetNode = node;
                fileBrowserTargetBuffer = buffer;
            }

            ImVec2 loadButtonPos = ImVec2(buttonPos.x + buttonWidth + spacing, buttonPos.y);
            ImGui::SetCursorScreenPos(loadButtonPos);
            const std::string loadId = "Load";
            if (ImGui::Button(loadId.c_str(), ImVec2(buttonWidth, buttonHeight)))
            {
                node->Process();
            }
        }
        else
        {
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
            {
                node->SetParam(pin.name, std::filesystem::path(buffer));
            }
            ImGui::PopItemWidth();
        }
    }

    void NodeRenderer::RenderCustomNodeContent(Engine::Node *node, const ImVec2 &nodePos, const ImVec2 &nodeSize)
    {
        if (!node)
            return;

        auto strategy = CreateRenderingStrategy(node);
        strategy->RenderCustomContent(*node, nodePos, nodeSize, canvas_.GetZoomLevel());
    }

    NodeDimensions NodeRenderer::CalculateNodeDimensions(const std::vector<NodePin> &pins,
        float zoomLevel,
        const Engine::Node *node)
    {
        return NodeDimensionCalculator::CalculateNodeDimensions(pins, zoomLevel, node);
    }

    std::unique_ptr<NodeRenderingStrategy> NodeRenderer::CreateRenderingStrategy(const Engine::Node *node)
    {
        if (!node)
        {
            return std::make_unique<DefaultNodeRenderingStrategy>();
        }

        if (const auto *imageNode = dynamic_cast<const Engine::ImageInputNode *>(node))
        {
            return std::make_unique<ImageInputNodeRenderingStrategy>();
        }

        if (const auto *previewNode = dynamic_cast<const Engine::PreviewNode *>(node))
        {
            return std::make_unique<PreviewNodeRenderingStrategy>();
        }

        return std::make_unique<DefaultNodeRenderingStrategy>();
    }

    void NodeRenderer::RenderFileBrowser()
    {
        if (fileBrowserOpen && !ImGuiFileDialog::Instance()->IsOpened("ChooseImageDlgKey"))
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ShowDevicesButton;

            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseImageDlgKey", "Choose Image File", "Image Files{.png,.jpg,.jpeg,.bmp,.tiff,.tif,.webp}", config);
            fileBrowserOpen = false;
        }

        ImVec2 minSize = ImVec2(800, 500);
        ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

        if (ImGuiFileDialog::Instance()->Display("ChooseImageDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string selectedPath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (!selectedPath.empty() && fileBrowserTargetNode && fileBrowserTargetBuffer)
                {
                    strncpy_s(fileBrowserTargetBuffer, 512, selectedPath.c_str(), 511);
                    fileBrowserTargetBuffer[511] = '\0';
                    fileBrowserTargetNode->SetParam("filepath", std::filesystem::path(selectedPath));
                    fileBrowserTargetNode->Process();
                }

                fileBrowserTargetNode = nullptr;
                fileBrowserTargetBuffer = nullptr;
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

} // namespace VisionCraft