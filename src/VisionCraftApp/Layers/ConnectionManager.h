#pragma once

#include "CanvasController.h"
#include "NodeEditor.h"
#include "NodeEditorConstants.h"
#include "NodeEditorTypes.h"

#include <imgui.h>
#include <unordered_map>
#include <vector>

/**
 * @file ConnectionManager.h
 * @brief Connection management component for the node editor.
 *
 * This class handles all connection-related operations including creation,
 * validation, rendering, and interaction handling. It follows the Single
 * Responsibility Principle by separating connection logic from the main node editor.
 */

namespace VisionCraft
{
    /**
     * @brief Component responsible for connection management in the node editor.
     *
     * The ConnectionManager handles:
     * - Connection creation and validation
     * - Connection rendering (bezier curves)
     * - Mouse interactions for connection dragging
     * - Pin position calculations
     * - Connection state management
     *
     * This component follows the Single Responsibility Principle and can be
     * easily tested in isolation or replaced with different connection behaviors.
     */
    class ConnectionManager
    {
    public:
        /**
         * @brief Default constructor.
         */
        ConnectionManager();

        /**
         * @brief Virtual destructor for potential inheritance.
         */
        virtual ~ConnectionManager();

        /**
         * @brief Handles mouse interactions for connection creation.
         * @param nodeEditor Reference to the node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Reference to canvas controller
         */
        void HandleConnectionInteractions(const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas);

        /**
         * @brief Renders all connections and connection preview.
         * @param nodeEditor Reference to the node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Reference to canvas controller
         */
        void RenderConnections(const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas);

        /**
         * @brief Creates a connection between two pins.
         * @param outputPin Source pin (must be output)
         * @param inputPin Target pin (must be input)
         * @param nodeEditor Reference to node editor for validation
         * @return True if connection was created successfully
         */
        bool CreateConnection(const PinId &outputPin, const PinId &inputPin, const Engine::NodeEditor &nodeEditor);

        /**
         * @brief Validates if a connection between two pins is allowed.
         * @param outputPin Source pin (must be output)
         * @param inputPin Target pin (must be input)
         * @param nodeEditor Reference to node editor for pin validation
         * @return True if connection is valid
         */
        bool IsConnectionValid(const PinId &outputPin,
            const PinId &inputPin,
            const Engine::NodeEditor &nodeEditor) const;

        /**
         * @brief Finds a pin at the given mouse position.
         * @param mousePos Mouse position in screen coordinates
         * @param nodeEditor Reference to the node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Reference to canvas controller
         * @return PinId if found, otherwise invalid PinId (nodeId = -1)
         */
        PinId FindPinAtPosition(const ImVec2 &mousePos,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Gets the world position of a specific pin.
         * @param pinId The pin to locate
         * @param nodeEditor Reference to the node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Reference to canvas controller
         * @return World position of the pin center
         */
        ImVec2 GetPinWorldPosition(const PinId &pinId,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Gets all active connections.
         * @return Vector of all connections
         */
        const std::vector<NodeConnection> &GetConnections() const;

        /**
         * @brief Gets the current connection state.
         * @return Current connection creation state
         */
        const ConnectionState &GetConnectionState() const;

        /**
         * @brief Gets the pins for a specific node type.
         * @param nodeType Type of node
         * @return Vector of pins for the node
         */
        static std::vector<NodePin> GetNodePins(const std::string &nodeType);

        /**
         * @brief Calculates node dimensions based on pins and zoom level.
         * @param pins Vector of node pins
         * @param zoomLevel Current zoom level
         * @return Calculated node dimensions
         */
        static NodeDimensions CalculateNodeDimensions(const std::vector<NodePin> &pins, float zoomLevel);

        /**
         * @brief Check if currently creating a connection.
         * @return True if in connection creation mode
         */
        bool IsCreatingConnection() const;

        /**
         * @brief Get the starting pin of current connection being created.
         * @return PinId of start pin, or invalid pin if not creating connection
         */
        const PinId &GetStartPin() const;

    private:
        /**
         * @brief Removes any existing connection to the given input pin.
         * @param inputPin The input pin to disconnect
         */
        void RemoveConnectionToInput(const PinId &inputPin);

        /**
         * @brief Renders a single connection as a bezier curve.
         * @param connection The connection to render
         * @param nodeEditor Reference to the node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Reference to canvas controller
         */
        void RenderConnection(const NodeConnection &connection,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas);

        // Connection data
        std::vector<NodeConnection> connections; ///< All active connections
        ConnectionState connectionState;         ///< Current connection creation state
    };

} // namespace VisionCraft