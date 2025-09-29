#pragma once

#include "CanvasController.h"
#include "ConnectionManager.h"
#include "Node.h"
#include "NodeEditorTypes.h"
#include "NodeRenderingStrategy.h"

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
     * @brief NodeRenderer handles the visual rendering of nodes in a column-based layout.
     *
     * This class is responsible for rendering nodes with input parameters in one column
     * and output parameters in another column, with proper scaling and boundary constraints.
     */
    class NodeRenderer
    {
    public:
        /**
         * @brief Constructor.
         * @param canvas Reference to the canvas controller
         * @param connectionManager Reference to the connection manager
         */
        NodeRenderer(CanvasController &canvas, ConnectionManager &connectionManager);

        /**
         * @brief Renders a single node with column-based parameter layout.
         * @param node Pointer to the node to render
         * @param nodePos Position of the node
         * @param selectedNodeId Currently selected node ID for highlighting
         * @param getPinInteractionState Function to get pin interaction state
         */
        void RenderNode(Engine::Node *node,
            const NodePosition &nodePos,
            Engine::NodeId selectedNodeId,
            std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState);

        /**
         * @brief Renders node parameters in a two-column layout.
         * @param node Pointer to the node
         * @param startPos Starting position for parameter rendering
         * @param nodeSize Size of the node
         * @param inputPins Input parameter pins
         * @param outputPins Output parameter pins
         */
        void RenderNodeParametersInColumns(Engine::Node *node,
            const ImVec2 &startPos,
            const ImVec2 &nodeSize,
            const std::vector<NodePin> &inputPins,
            const std::vector<NodePin> &outputPins);

        /**
         * @brief Renders pins in a column layout.
         * @param node Pointer to the node
         * @param pins Vector of pins to render
         * @param nodeWorldPos World position of the node
         * @param dimensions Node dimensions
         * @param isInputColumn True if rendering input pins, false for output pins
         * @param getPinInteractionState Function to get pin interaction state
         */
        void RenderPinsInColumn(Engine::Node *node,
            const std::vector<NodePin> &pins,
            const ImVec2 &nodeWorldPos,
            const NodeDimensions &dimensions,
            bool isInputColumn,
            std::function<PinInteractionState(Engine::NodeId, const std::string &)> getPinInteractionState);

    private:
        /**
         * @brief Renders a pin with its label text.
         * @param pin The pin to render
         * @param pinPos Screen position of the pin
         * @param labelPos Screen position of the label
         * @param pinRadius Radius of the pin circle
         * @param zoomLevel Current zoom level
         * @param state Pin interaction state (hover/active)
         */
        void RenderPinWithLabel(const NodePin &pin,
            const ImVec2 &pinPos,
            const ImVec2 &labelPos,
            float pinRadius,
            float zoomLevel,
            const PinInteractionState &state) const;

        /**
         * @brief Renders a single pin (input or output).
         * @param pin The pin to render
         * @param position Screen position for the pin
         * @param radius Radius of the pin circle
         * @param state Pin interaction state (hover/active)
         */
        void RenderPin(const NodePin &pin,
            const ImVec2 &position,
            float radius,
            const PinInteractionState &state = {}) const;

        /**
         * @brief Gets the color for a specific data type.
         * @param dataType The data type
         * @return ImU32 color value
         */
        [[nodiscard]] ImU32 GetDataTypeColor(PinDataType dataType) const;

        /**
         * @brief Converts camelCase parameter names to Title Case for display.
         * @param paramName The parameter name in camelCase (e.g., "lowThreshold")
         * @return User-friendly display name (e.g., "Low Threshold")
         */
        [[nodiscard]] static std::string FormatParameterName(const std::string &paramName);

        /**
         * @brief Calculates the layout for column-based parameter rendering.
         * @param nodeSize Size of the node
         * @param inputPinCount Number of input pins
         * @param outputPinCount Number of output pins
         * @return Calculated column dimensions and positions
         */
        struct ColumnLayout
        {
            float leftColumnX;
            float rightColumnX;
            float columnWidth;
            float rowHeight;
            float maxRows;
        };
        [[nodiscard]] ColumnLayout
            CalculateColumnLayout(const ImVec2 &nodeSize, size_t inputPinCount, size_t outputPinCount) const;

        /**
         * @brief Ensures text fits within column boundaries by truncating if necessary.
         * @param text Original text
         * @param maxWidth Maximum width allowed
         * @return Truncated text that fits within bounds
         */
        [[nodiscard]] std::string FitTextInColumn(const std::string &text, float maxWidth) const;


        /**
         * @brief Renders a single parameter within a column with proper bounds checking.
         * @param node Pointer to the node
         * @param pin The parameter pin to render
         * @param position Position to render the parameter
         * @param columnWidth Width of the column
         */
        void RenderParameterInColumn(Engine::Node *node, const NodePin &pin, const ImVec2 &position, float columnWidth);

        /**
         * @brief Renders an input widget next to an unconnected parameter pin.
         * @param node Pointer to the node
         * @param pin The parameter pin to render widget for
         * @param pinPos Position of the pin circle
         * @param pinRadius Radius of the pin circle
         */
        void RenderParameterInputWidget(Engine::Node *node, const NodePin &pin, const ImVec2 &pinPos, float pinRadius);

        /**
         * @brief Renders a string input widget.
         * @param node Pointer to the node
         * @param pin The parameter pin
         * @param widgetId Unique widget ID
         * @param inputWidth Width of the input widget
         */
        void RenderStringInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders a float input widget.
         * @param node Pointer to the node
         * @param pin The parameter pin
         * @param widgetId Unique widget ID
         * @param inputWidth Width of the input widget
         */
        void RenderFloatInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders an integer input widget.
         * @param node Pointer to the node
         * @param pin The parameter pin
         * @param widgetId Unique widget ID
         * @param inputWidth Width of the input widget
         */
        void RenderIntInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders a boolean checkbox widget.
         * @param node Pointer to the node
         * @param pin The parameter pin
         * @param widgetId Unique widget ID
         */
        void RenderBoolInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId);

        /**
         * @brief Renders a path input widget with optional file browser buttons.
         * @param node Pointer to the node
         * @param pin The parameter pin
         * @param widgetId Unique widget ID
         * @param inputWidth Width of the input widget
         */
        void RenderPathInput(Engine::Node *node, const NodePin &pin, const std::string &widgetId, float inputWidth);

        /**
         * @brief Renders node background and border.
         * @param worldPos Screen position of the node
         * @param nodeSize Size of the node
         * @param isSelected Whether the node is selected
         */
        void RenderNodeBackground(const ImVec2 &worldPos, const ImVec2 &nodeSize, bool isSelected);

        /**
         * @brief Renders node title bar background.
         * @param worldPos Screen position of the node
         * @param nodeSize Size of the node
         */
        void RenderNodeTitleBar(const ImVec2 &worldPos, const ImVec2 &nodeSize);

        /**
         * @brief Renders node title text.
         * @param node Pointer to the node
         * @param worldPos Screen position of the node
         */
        void RenderNodeTitleText(Engine::Node *node, const ImVec2 &worldPos);

        /**
         * @brief Separates pins into input and output vectors.
         * @param pins All pins for the node
         * @return Pair of (input pins, output pins)
         */
        [[nodiscard]] std::pair<std::vector<NodePin>, std::vector<NodePin>> SeparateInputOutputPins(
            const std::vector<NodePin> &pins);

        /**
         * @brief Renders custom node content (e.g., image preview for ImageInputNode).
         * @param node Pointer to the node
         * @param nodePos Position of the node
         * @param nodeSize Size of the node
         */
        void RenderCustomNodeContent(Engine::Node *node, const ImVec2 &nodePos, const ImVec2 &nodeSize);

    public:
        /**
         * @brief Calculates node dimensions based on pins and node type.
         * This is the proper place for dimension calculation - nodes know their own layout!
         * @param pins Vector of pins for this node
         * @param zoomLevel Current zoom level
         * @param node Pointer to the node (for node-specific sizing like ImageInputNode)
         * @return Calculated node dimensions
         */
        [[nodiscard]] static NodeDimensions CalculateNodeDimensions(const std::vector<NodePin> &pins,
            float zoomLevel,
            const Engine::Node *node = nullptr);

    private:
        /**
         * @brief Creates appropriate rendering strategy for the given node.
         * @param node The node to create strategy for
         * @return Unique pointer to the appropriate strategy
         */
        [[nodiscard]] static std::unique_ptr<NodeRenderingStrategy> CreateRenderingStrategy(const Engine::Node *node);
        CanvasController &canvas_;
        ConnectionManager &connectionManager_;
    };

} // namespace VisionCraft