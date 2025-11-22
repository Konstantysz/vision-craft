#pragma once

#include "UI/Canvas/CanvasController.h"
#include "UI/Canvas/ConnectionManager.h"
#include "UI/Rendering/Strategies/NodeRenderingStrategy.h"
#include "UI/Widgets/NodeEditorTypes.h"
#include "Nodes/Core/Node.h"

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>

namespace VisionCraft::UI::Rendering
{
    /**
     * @brief Pin interaction state result.
     */
    struct PinInteractionState
    {
        bool isHovered = false;
        bool isActive = false;
    };

    /**
     * @brief Renders nodes with column-based layout.
     */
    class NodeRenderer
    {
    public:
        /**
         * @brief Constructs renderer.
         * @param canvas Canvas controller
         * @param connectionManager Connection manager
         */
        NodeRenderer(Canvas::CanvasController &canvas, Canvas::ConnectionManager &connectionManager);

        /**
         * @brief Renders node.
         * @param node Nodes::Node to render
         * @param nodePos Nodes::Node position
         * @param selectedNodeId Selected node ID
         * @param getPinInteractionState Pin interaction state function
         */
        void RenderNode(Nodes::Node *node,
            const Widgets::NodePosition &nodePos,
            Nodes::NodeId selectedNodeId,
            std::function<PinInteractionState(Nodes::NodeId, const std::string &)> getPinInteractionState);

        /**
         * @brief Renders parameters in columns.
         * @param node Nodes::Node
         * @param startPos Start position
         * @param nodeSize Nodes::Node size
         * @param inputPins Input pins
         * @param outputPins Output pins
         */
        void RenderNodeParametersInColumns(Nodes::Node *node,
            const ImVec2 &startPos,
            const ImVec2 &nodeSize,
            const std::vector<Widgets::NodePin> &inputPins,
            const std::vector<Widgets::NodePin> &outputPins);

        /**
         * @brief Renders pins in column.
         * @param node Nodes::Node
         * @param pins Pins to render
         * @param nodeWorldPos Nodes::Node world position
         * @param dimensions Nodes::Node dimensions
         * @param isInputColumn True if input column
         * @param getPinInteractionState Pin interaction state function
         */
        void RenderPinsInColumn(Nodes::Node *node,
            const std::vector<Widgets::NodePin> &pins,
            const ImVec2 &nodeWorldPos,
            const Widgets::NodeDimensions &dimensions,
            bool isInputColumn,
            bool hasExecutionPins,
            std::function<PinInteractionState(Nodes::NodeId, const std::string &)> getPinInteractionState);

        /**
         * @brief Renders execution pins in a horizontal row at the top of the node.
         * @param node Node
         * @param executionInputPins Execution input pins
         * @param executionOutputPins Execution output pins
         * @param nodeWorldPos Node world position
         * @param dimensions Node dimensions
         * @param getPinInteractionState Pin interaction state function
         */
        void RenderExecutionPinRow(Nodes::Node *node,
            const std::vector<Widgets::NodePin> &executionInputPins,
            const std::vector<Widgets::NodePin> &executionOutputPins,
            const ImVec2 &nodeWorldPos,
            const Widgets::NodeDimensions &dimensions,
            std::function<PinInteractionState(Nodes::NodeId, const std::string &)> getPinInteractionState);

    private:
        /**
         * @brief Renders pin with label.
         * @param pin Pin
         * @param pinPos Pin position
         * @param labelPos Label position
         * @param pinRadius Pin radius
         * @param zoomLevel Zoom level
         * @param state Interaction state
         */
        void RenderPinWithLabel(const Widgets::NodePin &pin,
            const ImVec2 &pinPos,
            const ImVec2 &labelPos,
            float pinRadius,
            float zoomLevel,
            const PinInteractionState &state) const;

        /**
         * @brief Renders pin.
         * @param pin Pin
         * @param position Position
         * @param radius Radius
         * @param state Interaction state
         */
        void RenderPin(const Widgets::NodePin &pin,
            const ImVec2 &position,
            float radius,
            const PinInteractionState &state = {}) const;

        /**
         * @brief Returns data type color.
         * @param dataType Data type
         * @return Color
         */
        [[nodiscard]] ImU32 GetDataTypeColor(Widgets::PinDataType dataType) const;

        /**
         * @brief Formats parameter name for display.
         * @param paramName Parameter name
         * @return Formatted name
         */
        [[nodiscard]] static std::string FormatParameterName(const std::string &paramName);

        /**
         * @brief Column layout result.
         */
        struct ColumnLayout
        {
            float leftColumnX;
            float rightColumnX;
            float columnWidth;
            float rowHeight;
            float maxRows;
        };

        /**
         * @brief Calculates column layout.
         * @param nodeSize Nodes::Node size
         * @param inputPinCount Input pin count
         * @param outputPinCount Output pin count
         * @return Column layout
         */
        [[nodiscard]] ColumnLayout
            CalculateColumnLayout(const ImVec2 &nodeSize, size_t inputPinCount, size_t outputPinCount) const;

        /**
         * @brief Fits text in column.
         * @param text Text
         * @param maxWidth Maximum width
         * @return Fitted text
         */
        [[nodiscard]] std::string FitTextInColumn(const std::string &text, float maxWidth) const;

        /**
         * @brief Renders parameter in column.
         * @param node Nodes::Node
         * @param pin Pin
         * @param position Position
         * @param columnWidth Column width
         */
        void RenderParameterInColumn(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const ImVec2 &position,
            float columnWidth);

        /**
         * @brief Renders parameter input widget.
         * @param node Nodes::Node
         * @param pin Pin
         * @param pinPos Pin position
         * @param pinRadius Pin radius
         */
        void RenderParameterInputWidget(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const ImVec2 &pinPos,
            float pinRadius);

        /**
         * @brief Renders string input.
         * @param node Nodes::Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderStringInput(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const std::string &widgetId,
            float inputWidth);

        /**
         * @brief Renders float input.
         * @param node Nodes::Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderFloatInput(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const std::string &widgetId,
            float inputWidth);

        /**
         * @brief Renders integer input.
         * @param node Nodes::Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderIntInput(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const std::string &widgetId,
            float inputWidth);

        /**
         * @brief Renders boolean input.
         * @param node Nodes::Node
         * @param pin Pin
         * @param widgetId Widget ID
         */
        void RenderBoolInput(Nodes::Node *node, const Widgets::NodePin &pin, const std::string &widgetId);

        /**
         * @brief Renders path input.
         * @param node Nodes::Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderPathInput(Nodes::Node *node,
            const Widgets::NodePin &pin,
            const std::string &widgetId,
            float inputWidth);

        /**
         * @brief Renders node background.
         * @param worldPos World position
         * @param nodeSize Nodes::Node size
         * @param isSelected Whether selected
         */
        void RenderNodeBackground(const ImVec2 &worldPos, const ImVec2 &nodeSize, bool isSelected);

        /**
         * @brief Renders title bar.
         * @param worldPos World position
         * @param nodeSize Nodes::Node size
         */
        void RenderNodeTitleBar(const ImVec2 &worldPos, const ImVec2 &nodeSize);

        /**
         * @brief Renders title text.
         * @param node Nodes::Node
         * @param worldPos World position
         */
        void RenderNodeTitleText(Nodes::Node *node, const ImVec2 &worldPos);

        /**
         * @brief Separates pins into input and output.
         * @param pins All pins
         * @return Input and output pins
         */
        [[nodiscard]] std::pair<std::vector<Widgets::NodePin>, std::vector<Widgets::NodePin>> SeparateInputOutputPins(
            const std::vector<Widgets::NodePin> &pins);

        /**
         * @brief Renders custom node content.
         * @param node Nodes::Node
         * @param nodePos Nodes::Node position
         * @param nodeSize Nodes::Node size
         */
        void RenderCustomNodeContent(Nodes::Node *node, const ImVec2 &nodePos, const ImVec2 &nodeSize);

    public:
        /**
         * @brief Calculates node dimensions.
         * @param pins Pins
         * @param zoomLevel Zoom level
         * @param node Nodes::Node
         * @return Nodes::Node dimensions
         */
        [[nodiscard]] static Widgets::NodeDimensions CalculateNodeDimensions(const std::vector<Widgets::NodePin> &pins,
            float zoomLevel,
            const Nodes::Node *node = nullptr);

    private:
        /**
         * @brief Creates rendering strategy.
         * @param node Nodes::Node
         * @return Rendering strategy
         */
        [[nodiscard]] static std::unique_ptr<Rendering::Strategies::NodeRenderingStrategy> CreateRenderingStrategy(
            const Nodes::Node *node);

        /**
         * @brief Renders file browser dialog.
         */
        void RenderFileBrowser();

        Canvas::CanvasController &canvas_;
        Canvas::ConnectionManager &connectionManager_;

        bool fileBrowserOpen = false;
        Nodes::Node *fileBrowserTargetNode = nullptr;
        char *fileBrowserTargetBuffer = nullptr;
    };

} // namespace VisionCraft::UI::Rendering
