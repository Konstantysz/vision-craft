#pragma once

#include "Canvas/CanvasController.h"
#include "Connections/ConnectionManager.h"
#include "Editor/NodeEditorTypes.h"
#include "Rendering/Strategies/NodeRenderingStrategy.h"
#include "Node.h"

#include <functional>
#include <string>
#include <vector>

#include <imgui.h>

namespace VisionCraft
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
        NodeRenderer(CanvasController &canvas, ConnectionManager &connectionManager);

        /**
         * @brief Renders node.
         * @param node Node to render
         * @param nodePos Node position
         * @param selectedNodeId Selected node ID
         * @param getPinInteractionState Pin interaction state function
         */
        void RenderNode(Engine::Node *node,
            const NodePosition &nodePos,
            Engine::NodeId selectedNodeId,
            std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState);

        /**
         * @brief Renders parameters in columns.
         * @param node Node
         * @param startPos Start position
         * @param nodeSize Node size
         * @param inputPins Input pins
         * @param outputPins Output pins
         */
        void RenderNodeParametersInColumns(Engine::Node *node,
            const ImVec2 &startPos,
            const ImVec2 &nodeSize,
            const std::vector<NodePin> &inputPins,
            const std::vector<NodePin> &outputPins);

        /**
         * @brief Renders pins in column.
         * @param node Node
         * @param pins Pins to render
         * @param nodeWorldPos Node world position
         * @param dimensions Node dimensions
         * @param isInputColumn True if input column
         * @param getPinInteractionState Pin interaction state function
         */
        void RenderPinsInColumn(Engine::Node *node,
            const std::vector<NodePin> &pins,
            const ImVec2 &nodeWorldPos,
            const NodeDimensions &dimensions,
            bool isInputColumn,
            std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState);

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
        void RenderPinWithLabel(const NodePin &pin,
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
        void RenderPin(const NodePin &pin,
            const ImVec2 &position,
            float radius,
            const PinInteractionState &state = {}) const;

        /**
         * @brief Returns data type color.
         * @param dataType Data type
         * @return Color
         */
        [[nodiscard]] ImU32 GetDataTypeColor(PinDataType dataType) const;

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
         * @param nodeSize Node size
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
         * @param node Node
         * @param pin Pin
         * @param position Position
         * @param columnWidth Column width
         */
        void RenderParameterInColumn(Engine::Node *node, const NodePin &pin, const ImVec2 &position, float columnWidth);

        /**
         * @brief Renders parameter input widget.
         * @param node Node
         * @param pin Pin
         * @param pinPos Pin position
         * @param pinRadius Pin radius
         */
        void RenderParameterInputWidget(Engine::Node *node, const NodePin &pin, const ImVec2 &pinPos, float pinRadius);

        /**
         * @brief Renders string input.
         * @param node Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderStringInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders float input.
         * @param node Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderFloatInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders integer input.
         * @param node Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderIntInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders boolean input.
         * @param node Node
         * @param pin Pin
         * @param widgetId Widget ID
         */
        void RenderBoolInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId);

        /**
         * @brief Renders path input.
         * @param node Node
         * @param pin Pin
         * @param widgetId Widget ID
         * @param inputWidth Input width
         */
        void RenderPathInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders node background.
         * @param worldPos World position
         * @param nodeSize Node size
         * @param isSelected Whether selected
         */
        void RenderNodeBackground(const ImVec2 &worldPos, const ImVec2 &nodeSize, bool isSelected);

        /**
         * @brief Renders title bar.
         * @param worldPos World position
         * @param nodeSize Node size
         */
        void RenderNodeTitleBar(const ImVec2 &worldPos, const ImVec2 &nodeSize);

        /**
         * @brief Renders title text.
         * @param node Node
         * @param worldPos World position
         */
        void RenderNodeTitleText(Engine::Node *node, const ImVec2 &worldPos);

        /**
         * @brief Separates pins into input and output.
         * @param pins All pins
         * @return Input and output pins
         */
        [[nodiscard]] std::pair<std::vector<NodePin>, std::vector<NodePin>> SeparateInputOutputPins(
            const std::vector<NodePin> &pins);

        /**
         * @brief Renders custom node content.
         * @param node Node
         * @param nodePos Node position
         * @param nodeSize Node size
         */
        void RenderCustomNodeContent(Engine::Node *node, const ImVec2 &nodePos, const ImVec2 &nodeSize);

    public:
        /**
         * @brief Calculates node dimensions.
         * @param pins Pins
         * @param zoomLevel Zoom level
         * @param node Node
         * @return Node dimensions
         */
        [[nodiscard]] static NodeDimensions CalculateNodeDimensions(const std::vector<NodePin> &pins,
            float zoomLevel,
            const Engine::Node *node = nullptr);

    private:
        /**
         * @brief Creates rendering strategy.
         * @param node Node
         * @return Rendering strategy
         */
        [[nodiscard]] static std::unique_ptr<NodeRenderingStrategy> CreateRenderingStrategy(const Engine::Node *node);
        CanvasController &canvas_;
        ConnectionManager &connectionManager_;

        /**
         * @brief Renders file browser dialog.
         */
        void RenderFileBrowser();

        bool fileBrowserOpen = false;
        Engine::Node *fileBrowserTargetNode = nullptr;
        char *fileBrowserTargetBuffer = nullptr;
    };

} // namespace VisionCraft
