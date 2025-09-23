#pragma once

#include "Layer.h"
#include "NodeEditor.h"
#include "CanvasController.h"

#include <memory>
#include <unordered_map>

#include <imgui.h>

namespace VisionCraft
{
    /**
     * @brief Structure representing the visual position of a node in the editor.
     */
    struct NodePosition
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    /**
     * @brief Enum for different data types in the node editor.
     */
    enum class PinDataType
    {
        Image,  // cv::Mat - Green
        String, // std::string - Magenta
        Float,  // double - Light Blue
        Int,    // int - Cyan
        Bool    // bool - Red
    };

    /**
     * @brief Structure representing an input or output pin on a node.
     */
    struct NodePin
    {
        std::string name;
        PinDataType dataType;
        bool isInput;
    };

    /**
     * @brief Structure containing pre-calculated node dimensions.
     */
    struct NodeDimensions
    {
        ImVec2 size;
        size_t inputPinCount;
        size_t outputPinCount;
        size_t parameterPinCount;
    };

    /**
     * @brief Unique identifier for a pin on a node.
     */
    struct PinId
    {
        Engine::NodeId nodeId;
        std::string pinName;

        bool operator==(const PinId &other) const
        {
            return nodeId == other.nodeId && pinName == other.pinName;
        }

        bool operator<(const PinId &other) const
        {
            if (nodeId != other.nodeId)
                return nodeId < other.nodeId;
            return pinName < other.pinName;
        }
    };

    /**
     * @brief Represents a connection between two pins.
     * Note: Each input pin can only have ONE connection, but output pins can have multiple connections.
     */
    struct NodeConnection
    {
        PinId outputPin; // Source pin (must be output)
        PinId inputPin;  // Target pin (must be input, can only have one connection)

        bool operator==(const NodeConnection &other) const
        {
            return outputPin == other.outputPin && inputPin == other.inputPin;
        }
    };

    /**
     * @brief Connection creation state.
     */
    struct ConnectionState
    {
        bool isCreating = false;
        PinId startPin;
        ImVec2 endPosition;
    };

    /**
     * @brief Node editor layer.
     *
     * NodeEditorLayer implements the complete node editing experience combining
     * canvas management (pan/zoom/grid) with node rendering and interactions.
     * Similar to UE Blueprints, everything is contained in a single unified interface.
     */
    class NodeEditorLayer : public Core::Layer
    {
    public:
        /**
         * @brief Default constructor.
         */
        NodeEditorLayer() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~NodeEditorLayer() = default;

        /**
         * @brief Handles canvas input events.
         * @param event The event to handle (mouse, keyboard, etc.)
         *
         * Processes mouse wheel events for zooming and mouse drag events for panning.
         */
        void OnEvent(Core::Event &event) override;

        /**
         * @brief Updates the canvas state.
         * @param deltaTime Time elapsed since the last update
         */
        void OnUpdate(float deltaTime) override;

        /**
         * @brief Renders the canvas and grid.
         *
         * Draws the infinite grid background and provides the drawing surface
         * for the node editor. Uses ImGui's draw list for custom rendering.
         */
        void OnRender() override;

    private:
        /**
         * @brief Renders all nodes in the editor.
         */
        void RenderNodes();

        /**
         * @brief Renders a single node.
         * @param node Pointer to the node to render
         * @param nodePos Position of the node
         */
        void RenderNode(Engine::Node *node, const NodePosition &nodePos);

        /**
         * @brief Tests if a mouse position hits a node.
         * @param mousePos Mouse position in screen coordinates
         * @param nodePos Node position in world coordinates
         * @param nodeSize Node size in pixels
         * @return True if mouse hits the node
         */
        bool IsMouseOverNode(const ImVec2 &mousePos, const NodePosition &nodePos, const ImVec2 &nodeSize) const;

        /**
         * @brief Handles mouse interactions for node selection and dragging.
         */
        void HandleMouseInteractions();

        /**
         * @brief Renders the context menu for creating nodes.
         */
        void RenderContextMenu();

        /**
         * @brief Creates a new node of the specified type at the given position.
         * @param nodeType Type of node to create
         * @param position Position to place the node
         */
        void CreateNodeAtPosition(const std::string &nodeType, const ImVec2 &position);

        /**
         * @brief Gets the pins for a specific node type.
         * @param nodeType Type of node
         * @return Vector of pins for the node
         */
        static std::vector<NodePin> GetNodePins(const std::string &nodeType);

        /**
         * @brief Gets the color for a specific data type.
         * @param dataType The data type
         * @return ImU32 color value
         */
        ImU32 GetDataTypeColor(PinDataType dataType) const;

        /**
         * @brief Renders a single pin (input or output).
         * @param pin The pin to render
         * @param position Screen position for the pin
         * @param radius Radius of the pin circle
         */
        void RenderPin(const NodePin &pin, const ImVec2 &position, float radius) const;

        /**
         * @brief Renders all pins for a node.
         * @param pins Vector of all pins for the node
         * @param nodeWorldPos World position of the node
         * @param dimensions Pre-calculated node dimensions
         * @param zoomLevel Current zoom level
         */
        void RenderNodePins(const std::vector<NodePin> &pins,
            const ImVec2 &nodeWorldPos,
            const NodeDimensions &dimensions,
            float zoomLevel);

        /**
         * @brief Gets the editable parameters for a specific node type.
         * @param nodeName Name of the node
         * @return Vector of parameter definitions
         */
        std::vector<NodePin> GetNodeParameters(const std::string &nodeName) const;

        /**
         * @brief Renders inline parameter editors for a node.
         * @param node Pointer to the node
         * @param startPos Starting position for parameter rendering
         * @param nodeSize Size of the node
         */
        void RenderNodeParameters(Engine::Node *node, const ImVec2 &startPos, const ImVec2 &nodeSize);

        /**
         * @brief Converts camelCase parameter names to Title Case for display.
         * @param paramName The parameter name in camelCase (e.g., "lowThreshold")
         * @return User-friendly display name (e.g., "Low Threshold")
         */
        static std::string FormatParameterName(const std::string &paramName);

        /**
         * @brief Finds the topmost node at the given mouse position.
         * @param mousePos Mouse position in screen coordinates
         * @return Node ID if found, or -1 if no node at position
         */
        Engine::NodeId FindNodeAtPosition(const ImVec2 &mousePos) const;

        /**
         * @brief Finds a pin at the given mouse position.
         * @param mousePos Mouse position in screen coordinates
         * @return PinId if found, otherwise invalid PinId (nodeId = -1)
         */
        PinId FindPinAtPosition(const ImVec2 &mousePos) const;

        /**
         * @brief Gets the world position of a specific pin.
         * @param pinId The pin to locate
         * @return World position of the pin center
         */
        ImVec2 GetPinWorldPosition(const PinId &pinId) const;

        /**
         * @brief Validates if a connection between two pins is allowed.
         * @param outputPin Source pin (must be output)
         * @param inputPin Target pin (must be input)
         * @return True if connection is valid
         */
        bool IsConnectionValid(const PinId &outputPin, const PinId &inputPin) const;

        /**
         * @brief Creates a connection between two pins.
         * @param outputPin Source pin (must be output)
         * @param inputPin Target pin (must be input)
         * @return True if connection was created successfully
         */
        bool CreateConnection(const PinId &outputPin, const PinId &inputPin);

        /**
         * @brief Removes any existing connection to the given input pin.
         * @param inputPin The input pin to disconnect
         */
        void RemoveConnectionToInput(const PinId &inputPin);

        /**
         * @brief Handles mouse interactions for connection creation.
         */
        void HandleConnectionInteractions();

        /**
         * @brief Renders all connections.
         */
        void RenderConnections();

        /**
         * @brief Renders a single connection as a bezier curve.
         * @param connection The connection to render
         */
        void RenderConnection(const NodeConnection &connection);

        // Core components
        CanvasController canvas;                                         ///< Canvas management component
        Engine::NodeEditor nodeEditor;                                  ///< Backend node editor
        std::unordered_map<Engine::NodeId, NodePosition> nodePositions; ///< Visual positions of nodes
        Engine::NodeId nextNodeId = 1;                                  ///< Next available node ID

        // Selection and dragging state
        Engine::NodeId selectedNodeId = Constants::Special::kInvalidNodeId; ///< Currently selected node ID
        bool isDragging = false;                ///< Whether a node is being dragged
        ImVec2 dragOffset = ImVec2(0.0f, 0.0f); ///< Mouse offset from node position during drag

        // Context menu state
        bool showContextMenu = false;                 ///< Whether to show the context menu
        ImVec2 contextMenuPos = ImVec2(0.0f, 0.0f);   ///< Position where context menu was opened

        // Connection state
        std::vector<NodeConnection> connections; ///< All active connections
        ConnectionState connectionState;         ///< Current connection creation state
    };
} // namespace VisionCraft