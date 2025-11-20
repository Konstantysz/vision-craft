#include "UI/Rendering/NodeRenderer.h"
#include "UI/Rendering/NodeDimensionCalculator.h"
#include "UI/Rendering/Strategies/DefaultNodeRenderingStrategy.h"
#include "UI/Rendering/Strategies/ImageInputNodeRenderingStrategy.h"
#include "UI/Rendering/Strategies/PreviewNodeRenderingStrategy.h"
#include "UI/Widgets/NodeEditorConstants.h"
#include "Logger.h"
#include "Vision/IO/ImageInputNode.h"
#include "Vision/IO/PreviewNode.h"

#include <ImGuiFileDialog.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iterator>

namespace VisionCraft::UI::Rendering
{
    NodeRenderer::NodeRenderer(Canvas::CanvasController &canvas, Canvas::ConnectionManager &connectionManager)
        : canvas_(canvas), connectionManager_(connectionManager)
    {
    }

    void NodeRenderer::RenderNode(Nodes::Node *node,
        const Widgets::NodePosition &nodePos,
        Nodes::NodeId selectedNodeId,
        std::function<PinInteractionState(Nodes::NodeId, const std::string &)> getPinInteractionState)
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

    void NodeRenderer::RenderNodeTitleText(Nodes::Node *node, const ImVec2 &worldPos)
    {
        if (canvas_.GetZoomLevel() > Constants::Zoom::kMinForText)
        {
            auto *drawList = ImGui::GetWindowDrawList();
            const auto textPos = ImVec2(worldPos.x + Constants::Node::kPadding * canvas_.GetZoomLevel(),
                worldPos.y + Constants::Node::Text::kOffset * canvas_.GetZoomLevel());
            drawList->AddText(textPos, Constants::Colors::Node::kText, node->GetName().c_str());
        }
    }

    std::pair<std::vector<Widgets::NodePin>, std::vector<Widgets::NodePin>> NodeRenderer::SeparateInputOutputPins(
        const std::vector<Widgets::NodePin> &pins)
    {
        std::vector<Widgets::NodePin> inputPins;
        std::vector<Widgets::NodePin> outputPins;

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

    void NodeRenderer::RenderNodeParametersInColumns(Nodes::Node *node,
        const ImVec2 &startPos,
        const ImVec2 &nodeSize,
        const std::vector<Widgets::NodePin> &inputPins,
        const std::vector<Widgets::NodePin> &outputPins)
    {
        if (inputPins.empty() && outputPins.empty())
        {
            return;
        }

        const auto layout = CalculateColumnLayout(nodeSize, inputPins.size(), outputPins.size());
        auto *drawList = ImGui::GetWindowDrawList();
        auto *drawList = ImGui::GetWindowDrawList();
        // paramHeight and paramSpacing were unused
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

    void NodeRenderer::RenderPinsInColumn(Nodes::Node *node,
        const std::vector<Widgets::NodePin> &pins,
        const ImVec2 &nodeWorldPos,
        const Widgets::NodeDimensions &dimensions,
        bool isInputColumn,
        std::function<PinInteractionState(Nodes::NodeId, const std::string &)> getPinInteractionState)
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

    void NodeRenderer::RenderPinWithLabel(const Widgets::NodePin &pin,
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

    void NodeRenderer::RenderPin(const Widgets::NodePin &pin,
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

    ImU32 NodeRenderer::GetDataTypeColor(Widgets::PinDataType dataType) const
    {
        switch (dataType)
        {
        case Widgets::PinDataType::Image:
            return Constants::Colors::Pin::kImage;
        case Widgets::PinDataType::String:
            return Constants::Colors::Pin::kString;
        case Widgets::PinDataType::Float:
            return Constants::Colors::Pin::kFloat;
        case Widgets::PinDataType::Int:
            return Constants::Colors::Pin::kInt;
        case Widgets::PinDataType::Bool:
            return Constants::Colors::Pin::kBool;
        case Widgets::PinDataType::Path:
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


    void NodeRenderer::RenderParameterInColumn(Nodes::Node *node,
        const Widgets::NodePin &pin,
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
        case Widgets::PinDataType::String:
            RenderStringInput(node, pin, widgetId, inputWidth);
            break;

        case Widgets::PinDataType::Float:
            RenderFloatInput(node, pin, widgetId, inputWidth);
            break;

        case Widgets::PinDataType::Int:
            RenderIntInput(node, pin, widgetId, inputWidth);
            break;

        case Widgets::PinDataType::Bool:
            RenderBoolInput(node, pin, widgetId);
            break;

        case Widgets::PinDataType::Path:
            RenderPathInput(node, pin, widgetId, inputWidth);
            break;

        default: {
            std::string paramValue = "Unknown";
            ImGui::Text("%s", FitTextInColumn(paramValue, inputWidth).c_str());
            break;
        }
        }
    }

    void NodeRenderer::RenderParameterInputWidget(Nodes::Node *node,
        const Widgets::NodePin &pin,
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
        case Widgets::PinDataType::String:
            RenderStringInput(node, pin, widgetId, inputWidth);
            break;
        case Widgets::PinDataType::Float:
            RenderFloatInput(node, pin, widgetId, inputWidth);
            break;
        case Widgets::PinDataType::Int:
            RenderIntInput(node, pin, widgetId, inputWidth);
            break;
        case Widgets::PinDataType::Bool:
            RenderBoolInput(node, pin, widgetId);
            break;
        case Widgets::PinDataType::Path:
            RenderPathInput(node, pin, widgetId, inputWidth);
            break;
        default:
            break;
        }
    }

    void NodeRenderer::RenderStringInput(Nodes::Node *node,
        const Widgets::NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        std::string paramValue = node->GetInputValue<std::string>(pin.name).value_or("");
        char buffer[256];
        std::strncpy(buffer, paramValue.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
        {
            node->SetInputSlotDefault(pin.name, std::string(buffer));
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderFloatInput(Nodes::Node *node,
        const Widgets::NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        float value = static_cast<float>(node->GetInputValue<double>(pin.name).value_or(
            Constants::NodeRenderer::ParameterInput::kDefaultFloatValue));

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputFloat(widgetId.c_str(),
                &value,
                Constants::NodeRenderer::ParameterInput::kFloatStep,
                Constants::NodeRenderer::ParameterInput::kFloatFastStep,
                Constants::NodeRenderer::ParameterInput::kFloatFormat))
        {
            node->SetInputSlotDefault(pin.name, static_cast<double>(value));
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderIntInput(Nodes::Node *node,
        const Widgets::NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        int value = node->GetInputValue<int>(pin.name).value_or(0);

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputInt(widgetId.c_str(), &value))
        {
            node->SetInputSlotDefault(pin.name, value);
        }
        ImGui::PopItemWidth();
    }

    void NodeRenderer::RenderBoolInput(Nodes::Node *node, const Widgets::NodePin &pin, const std::string &widgetId)
    {
        bool value = node->GetInputValue<bool>(pin.name).value_or(false);

        if (ImGui::Checkbox(widgetId.c_str(), &value))
        {
            node->SetInputSlotDefault(pin.name, value);
        }
    }

    void NodeRenderer::RenderPathInput(Nodes::Node *node,
        const Widgets::NodePin &pin,
        const std::string &widgetId,
        float inputWidth)
    {
        auto pathValue = node->GetInputValue<std::filesystem::path>(pin.name).value_or(std::filesystem::path{});
        std::string pathStr = pathValue.string();
        char buffer[256];
        std::strncpy(buffer, pathStr.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        bool isImageInputFilepath =
            (pin.name == "FilePath" && dynamic_cast<Vision::IO::ImageInputNode *>(node) != nullptr);

        if (isImageInputFilepath)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText(widgetId.c_str(), buffer, sizeof(buffer)))
            {
                node->SetInputSlotDefault(pin.name, std::filesystem::path(buffer));
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
                // Don't store buffer pointer - it's a local variable that will be invalid later!
                // We'll set the path directly via SetInputSlotDefault in the callback
                fileBrowserTargetBuffer = nullptr;
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
                node->SetInputSlotDefault(pin.name, std::filesystem::path(buffer));
            }
            ImGui::PopItemWidth();
        }
    }

    void NodeRenderer::RenderCustomNodeContent(Nodes::Node *node, const ImVec2 &nodePos, const ImVec2 &nodeSize)
    {
        if (!node)
            return;

        auto strategy = CreateRenderingStrategy(node);
        strategy->RenderCustomContent(*node, nodePos, nodeSize, canvas_.GetZoomLevel());
    }

    Widgets::NodeDimensions NodeRenderer::CalculateNodeDimensions(const std::vector<Widgets::NodePin> &pins,
        float zoomLevel,
        const Nodes::Node *node)
    {
        return NodeDimensionCalculator::CalculateNodeDimensions(pins, zoomLevel, node);
    }

    std::unique_ptr<Rendering::Strategies::NodeRenderingStrategy> NodeRenderer::CreateRenderingStrategy(
        const Nodes::Node *node)
    {
        if (!node)
        {
            return std::make_unique<Rendering::Strategies::DefaultNodeRenderingStrategy>();
        }

        if (const auto *imageNode = dynamic_cast<const Vision::IO::ImageInputNode *>(node))
        {
            return std::make_unique<Rendering::Strategies::ImageInputNodeRenderingStrategy>();
        }

        if (const auto *previewNode = dynamic_cast<const Vision::IO::PreviewNode *>(node))
        {
            return std::make_unique<Rendering::Strategies::PreviewNodeRenderingStrategy>();
        }

        return std::make_unique<Rendering::Strategies::DefaultNodeRenderingStrategy>();
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

                if (!selectedPath.empty() && fileBrowserTargetNode)
                {
                    fileBrowserTargetNode->SetInputSlotDefault("FilePath", std::filesystem::path(selectedPath));
                    fileBrowserTargetNode->Process();
                }

                fileBrowserTargetNode = nullptr;
                fileBrowserTargetBuffer = nullptr;
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

} // namespace VisionCraft::UI::Rendering
