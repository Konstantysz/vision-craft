#pragma once

#include "Editor/NodeEditorTypes.h"
#include "Command.h"
#include "NodeEditor.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace VisionCraft
{
    /**
     * @brief Command for creating a new node.
     */
    class CreateNodeCommand : public Command
    {
    public:
        /** @brief Function that creates a new node */
        using NodeCreator = std::function<std::unique_ptr<Engine::Node>()>;
        /** @brief Function that adds a node to the editor */
        using NodeAdder = std::function<void(std::unique_ptr<Engine::Node>)>;
        /** @brief Function that removes a node from the editor */
        using NodeRemover = std::function<void(Engine::NodeId)>;
        /** @brief Function that sets a node's position */
        using PositionSetter = std::function<void(Engine::NodeId, const NodePosition &)>;

        CreateNodeCommand(NodeCreator creator,
            NodeAdder adder,
            NodeRemover remover,
            PositionSetter positionSetter,
            const NodePosition &position,
            const std::string &nodeType);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        NodeCreator nodeCreator;           ///< Creates the node instance
        NodeAdder nodeAdder;               ///< Adds node to editor
        NodeRemover nodeRemover;           ///< Removes node from editor
        PositionSetter positionSetter;     ///< Sets node position
        NodePosition position;             ///< Target position for node
        std::string nodeType;              ///< Node type for description
        Engine::NodeId createdNodeId = -1; ///< ID of created node (set after Execute)
    };

    /**
     * @brief Command for deleting a node.
     */
    class DeleteNodeCommand : public Command
    {
    public:
        /** @brief Function that retrieves a node pointer */
        using NodeGetter = std::function<Engine::Node *(Engine::NodeId)>;
        /** @brief Function that removes a node from the editor */
        using NodeRemover = std::function<void(Engine::NodeId)>;
        /** @brief Function that adds a node to the editor */
        using NodeAdder = std::function<void(std::unique_ptr<Engine::Node>)>;
        /** @brief Function that retrieves a node's position */
        using PositionGetter = std::function<NodePosition(Engine::NodeId)>;
        /** @brief Function that sets a node's position */
        using PositionSetter = std::function<void(Engine::NodeId, const NodePosition &)>;
        /** @brief Function that recreates a node from type, id, and name */
        using NodeRecreator =
            std::function<std::unique_ptr<Engine::Node>(const std::string &, Engine::NodeId, const std::string &)>;

        DeleteNodeCommand(Engine::NodeId nodeId,
            NodeGetter nodeGetter,
            NodeRemover nodeRemover,
            NodeAdder nodeAdder,
            PositionGetter positionGetter,
            PositionSetter positionSetter,
            NodeRecreator nodeRecreator);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        Engine::NodeId nodeId;         ///< ID of node to delete
        NodeGetter nodeGetter;         ///< Gets node pointer
        NodeRemover nodeRemover;       ///< Removes node from editor
        NodeAdder nodeAdder;           ///< Adds node to editor
        PositionGetter positionGetter; ///< Gets node position
        PositionSetter positionSetter; ///< Sets node position
        NodeRecreator nodeRecreator;   ///< Recreates node from saved data

        std::string nodeType;      ///< Saved node type for undo
        std::string nodeName;      ///< Saved node name for undo
        NodePosition nodePosition; ///< Saved node position for undo
        bool executed = false;     ///< Tracks whether command has been executed
    };

    /**
     * @brief Command for moving one or more nodes.
     */
    class MoveNodesCommand : public Command
    {
    public:
        /** @brief Function that sets a node's position */
        using PositionSetter = std::function<void(Engine::NodeId, const NodePosition &)>;

        MoveNodesCommand(const std::unordered_map<Engine::NodeId, NodePosition> &oldPositions,
            const std::unordered_map<Engine::NodeId, NodePosition> &newPositions,
            PositionSetter positionSetter);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        std::unordered_map<Engine::NodeId, NodePosition> oldPositions; ///< Original positions before move
        std::unordered_map<Engine::NodeId, NodePosition> newPositions; ///< New positions after move
        PositionSetter positionSetter;                                 ///< Sets node positions
    };

} // namespace VisionCraft
