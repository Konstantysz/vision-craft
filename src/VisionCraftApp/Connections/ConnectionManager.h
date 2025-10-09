#pragma once

#include "Canvas/CanvasController.h"
#include "Editor/NodeEditorConstants.h"
#include "Editor/NodeEditorTypes.h"
#include "NodeEditor.h"

#include <functional>
#include <imgui.h>
#include <unordered_map>
#include <vector>

namespace VisionCraft
{
    /**
     * @brief Callback for connection creation.
     */
    using ConnectionCreatedCallback = std::function<void(const NodeConnection &)>;

    /**
     * @brief Connection management for creation, validation, and rendering.
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
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         */
        void HandleConnectionInteractions(Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas);

        /**
         * @brief Renders connections and connection preview.
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @param hoveredConnection Optional hovered connection for highlighting
         */
        void RenderConnections(const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas,
            const std::optional<NodeConnection> &hoveredConnection = std::nullopt);

        /**
         * @brief Creates connection between pins.
         * @param outputPin Source pin
         * @param inputPin Target pin
         * @param nodeEditor Node editor for validation
         * @param notifyCallback Whether to invoke the callback (default true)
         * @return True if created
         */
        bool CreateConnection(const PinId &outputPin,
            const PinId &inputPin,
            Engine::NodeEditor &nodeEditor,
            bool notifyCallback = true);

        /**
         * @brief Validates connection between pins.
         * @param outputPin Source pin
         * @param inputPin Target pin
         * @param nodeEditor Node editor for validation
         * @return True if valid
         */
        [[nodiscard]] bool IsConnectionValid(const PinId &outputPin,
            const PinId &inputPin,
            const Engine::NodeEditor &nodeEditor) const;

        /**
         * @brief Checks if pin is connected.
         * @param pin Pin to check
         * @return True if connected
         */
        [[nodiscard]] bool IsPinConnected(const PinId &pin) const;

        /**
         * @brief Checks if pin needs input widget.
         * @param nodeId Node ID
         * @param pin Pin to check
         * @return True if needs widget
         */
        [[nodiscard]] bool PinNeedsInputWidget(Engine::NodeId nodeId, const NodePin &pin) const;

        /**
         * @brief Finds pin at position.
         * @param mousePos Mouse position
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return PinId if found, invalid otherwise
         */
        [[nodiscard]] PinId FindPinAtPosition(const ImVec2 &mousePos,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Finds pin at position within specific node.
         * @param mousePos Mouse position
         * @param nodeId Node to search within
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return PinId if found, invalid otherwise
         */
        [[nodiscard]] PinId FindPinAtPositionInNode(const ImVec2 &mousePos,
            Engine::NodeId nodeId,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Returns pin world position.
         * @param pinId Pin to locate
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return Pin world position
         */
        [[nodiscard]] ImVec2 GetPinWorldPosition(const PinId &pinId,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Returns all connections.
         * @return Connections vector
         */
        [[nodiscard]] const std::vector<NodeConnection> &GetConnections() const;

        /**
         * @brief Returns connection creation state.
         * @return Connection state
         */
        [[nodiscard]] const ConnectionState &GetConnectionState() const;

        /**
         * @brief Returns pins for node type.
         * @param nodeType Node type
         * @return Pins vector
         */
        [[nodiscard]] static std::vector<NodePin> GetNodePins(const std::string &nodeType);

        /**
         * @brief Checks if creating connection.
         * @return True if creating
         */
        [[nodiscard]] bool IsCreatingConnection() const;

        /**
         * @brief Returns start pin of current connection.
         * @return Start pin
         */
        [[nodiscard]] const PinId &GetStartPin() const;

        /**
         * @brief Finds connection at mouse position.
         * @param mousePos Mouse position
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return Connection if found, otherwise invalid connection
         */
        [[nodiscard]] std::optional<NodeConnection> FindConnectionAtPosition(const ImVec2 &mousePos,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Removes a connection.
         * @param connection Connection to remove
         */
        void RemoveConnection(const NodeConnection &connection);

        /**
         * @brief Sets callback for connection creation.
         * @param callback Callback to invoke when connection is created
         */
        void SetConnectionCreatedCallback(ConnectionCreatedCallback callback);

    private:
        /**
         * @brief Removes connection to input pin.
         * @param inputPin Input pin to disconnect
         */
        void RemoveConnectionToInput(const PinId &inputPin);

        /**
         * @brief Renders single connection.
         * @param connection Connection to render
         * @param nodeEditor Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @param isHovered Whether connection is hovered
         */
        void RenderConnection(const NodeConnection &connection,
            const Engine::NodeEditor &nodeEditor,
            const std::unordered_map<Engine::NodeId, NodePosition> &nodePositions,
            const CanvasController &canvas,
            bool isHovered = false);

        // Connection data
        std::vector<NodeConnection> connections;       ///< All active connections
        ConnectionState connectionState;               ///< Current connection creation state
        ConnectionCreatedCallback onConnectionCreated; ///< Callback for connection creation
    };

} // namespace VisionCraft
