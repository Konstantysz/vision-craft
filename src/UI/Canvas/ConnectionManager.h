#pragma once

#include "UI/Canvas/CanvasController.h"
#include "UI/Widgets/NodeEditorConstants.h"
#include "UI/Widgets/NodeEditorTypes.h"
#include "Nodes/Core/NodeEditor.h"

#include <imgui.h>
#include <functional>
#include <unordered_map>
#include <vector>

namespace VisionCraft::UI::Canvas
{
    /**
     * @brief Callback for connection creation.
     */
    using ConnectionCreatedCallback = std::function<void(const UI::Widgets::NodeConnection &)>;

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
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         */
        void HandleConnectionInteractions(Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas);

        /**
         * @brief Renders connections and connection preview.
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @param hoveredConnection Optional hovered connection for highlighting
         */
        void RenderConnections(const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas,
            const std::optional<UI::Widgets::NodeConnection> &hoveredConnection = std::nullopt);

        /**
         * @brief Creates connection between pins.
         * @param outputPin Source pin
         * @param inputPin Target pin
         * @param nodeEditor Nodes::Node editor for validation
         * @param notifyCallback Whether to invoke the callback (default true)
         * @return True if created
         */
        bool CreateConnection(const UI::Widgets::PinId &outputPin,
            const UI::Widgets::PinId &inputPin,
            Nodes::NodeEditor &nodeEditor,
            bool notifyCallback = true);

        /**
         * @brief Validates connection between pins.
         * @param outputPin Source pin
         * @param inputPin Target pin
         * @param nodeEditor Nodes::Node editor for validation
         * @return True if valid
         */
        [[nodiscard]] bool IsConnectionValid(const UI::Widgets::PinId &outputPin,
            const UI::Widgets::PinId &inputPin,
            const Nodes::NodeEditor &nodeEditor) const;

        /**
         * @brief Checks if pin is connected.
         * @param pin Pin to check
         * @return True if connected
         */
        [[nodiscard]] bool IsPinConnected(const UI::Widgets::PinId &pin) const;

        /**
         * @brief Checks if pin needs input widget.
         * @param nodeId Nodes::Node ID
         * @param pin Pin to check
         * @return True if needs widget
         */
        [[nodiscard]] bool PinNeedsInputWidget(Nodes::NodeId nodeId, const UI::Widgets::NodePin &pin) const;

        /**
         * @brief Finds pin at position.
         * @param mousePos Mouse position
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return PinId if found, invalid otherwise
         */
        [[nodiscard]] UI::Widgets::PinId FindPinAtPosition(const ImVec2 &mousePos,
            const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Finds pin at position within specific node.
         * @param mousePos Mouse position
         * @param nodeId Nodes::Node to search within
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return PinId if found, invalid otherwise
         */
        [[nodiscard]] UI::Widgets::PinId FindPinAtPositionInNode(const ImVec2 &mousePos,
            Nodes::NodeId nodeId,
            const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Returns pin world position.
         * @param pinId Pin to locate
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return Pin world position
         */
        [[nodiscard]] ImVec2 GetPinWorldPosition(const UI::Widgets::PinId &pinId,
            const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Returns all connections.
         * @return Connections vector
         */
        [[nodiscard]] const std::vector<UI::Widgets::NodeConnection> &GetConnections() const;

        /**
         * @brief Returns connection creation state.
         * @return Connection state
         */
        [[nodiscard]] const UI::Widgets::ConnectionState &GetConnectionState() const;

        /**
         * @brief Returns pins for node type.
         * @param nodeType Nodes::Node type
         * @return Pins vector
         */
        [[nodiscard]] static std::vector<UI::Widgets::NodePin> GetNodePins(const std::string &nodeType);

        /**
         * @brief Checks if creating connection.
         * @return True if creating
         */
        [[nodiscard]] bool IsCreatingConnection() const;

        /**
         * @brief Returns start pin of current connection.
         * @return Start pin
         */
        [[nodiscard]] const UI::Widgets::PinId &GetStartPin() const;

        /**
         * @brief Finds connection at mouse position.
         * @param mousePos Mouse position
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @return Connection if found, otherwise invalid connection
         */
        [[nodiscard]] std::optional<UI::Widgets::NodeConnection> FindConnectionAtPosition(const ImVec2 &mousePos,
            const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas) const;

        /**
         * @brief Removes a connection.
         * @param connection Connection to remove
         */
        void RemoveConnection(const UI::Widgets::NodeConnection &connection);

        /**
         * @brief Removes all connections associated with a node.
         * @param nodeId ID of the node whose connections should be removed
         */
        void RemoveConnectionsForNode(Nodes::NodeId nodeId);

        /**
         * @brief Sets callback for connection creation.
         * @param callback Callback to invoke when connection is created
         */
        void SetConnectionCreatedCallback(ConnectionCreatedCallback callback);

        /**
         * @brief Rebuilds UI connections from NodeEditor data.
         * @param nodeEditor Node editor containing connection data
         *
         * This method synchronizes the ConnectionManager's UI connection list
         * with the connections stored in the NodeEditor. Call this after loading
         * a graph to ensure the UI reflects the loaded connections.
         */
        void RebuildConnectionsFromNodeEditor(const Nodes::NodeEditor &nodeEditor);

    private:
        /**
         * @brief Removes connection to input pin.
         * @param inputPin Input pin to disconnect
         */
        void RemoveConnectionToInput(const UI::Widgets::PinId &inputPin);

        /**
         * @brief Renders single connection.
         * @param connection Connection to render
         * @param nodeEditor Nodes::Node editor backend
         * @param nodePositions Map of node positions
         * @param canvas Canvas controller
         * @param isHovered Whether connection is hovered
         */
        void RenderConnection(const UI::Widgets::NodeConnection &connection,
            const Nodes::NodeEditor &nodeEditor,
            const std::unordered_map<Nodes::NodeId, UI::Widgets::NodePosition> &nodePositions,
            const CanvasController &canvas,
            bool isHovered = false);

        // Connection data
        std::vector<UI::Widgets::NodeConnection> connections; ///< All active connections
        UI::Widgets::ConnectionState connectionState;         ///< Current connection creation state
        ConnectionCreatedCallback onConnectionCreated;        ///< Callback for connection creation
    };

} // namespace VisionCraft::UI::Canvas
