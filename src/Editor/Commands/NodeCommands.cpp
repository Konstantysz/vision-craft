#include "NodeCommands.h"

namespace VisionCraft
{
    // CreateNodeCommand

    CreateNodeCommand::CreateNodeCommand(NodeCreator creator,
        NodeAdder adder,
        NodeRemover remover,
        PositionSetter positionSetter,
        const NodePosition &position,
        const std::string &nodeType)
        : nodeCreator(std::move(creator)), nodeAdder(std::move(adder)), nodeRemover(std::move(remover)),
          positionSetter(std::move(positionSetter)), position(position), nodeType(nodeType)
    {
    }

    void CreateNodeCommand::Execute()
    {
        auto node = nodeCreator();
        createdNodeId = node->GetId();
        nodeAdder(std::move(node));
        positionSetter(createdNodeId, position);
    }

    void CreateNodeCommand::Undo()
    {
        if (createdNodeId != -1)
        {
            nodeRemover(createdNodeId);
        }
    }

    std::string CreateNodeCommand::GetDescription() const
    {
        return "Create " + nodeType;
    }

    // DeleteNodeCommand

    DeleteNodeCommand::DeleteNodeCommand(Engine::NodeId nodeId,
        NodeGetter nodeGetter,
        NodeRemover nodeRemover,
        NodeAdder nodeAdder,
        PositionGetter positionGetter,
        PositionSetter positionSetter,
        NodeRecreator nodeRecreator)
        : nodeId(nodeId), nodeGetter(std::move(nodeGetter)), nodeRemover(std::move(nodeRemover)),
          nodeAdder(std::move(nodeAdder)), positionGetter(std::move(positionGetter)),
          positionSetter(std::move(positionSetter)), nodeRecreator(std::move(nodeRecreator))
    {
    }

    void DeleteNodeCommand::Execute()
    {
        if (!executed)
        {
            auto *node = nodeGetter(nodeId);
            if (node)
            {
                nodeType = node->GetType();
                nodeName = node->GetName();
                nodePosition = positionGetter(nodeId);
            }
        }

        nodeRemover(nodeId);
        executed = true;
    }

    void DeleteNodeCommand::Undo()
    {
        if (!executed)
        {
            return;
        }

        auto node = nodeRecreator(nodeType, nodeId, nodeName);
        if (node)
        {
            nodeAdder(std::move(node));
            positionSetter(nodeId, nodePosition);
        }
    }

    std::string DeleteNodeCommand::GetDescription() const
    {
        return "Delete Node";
    }

    // MoveNodesCommand

    MoveNodesCommand::MoveNodesCommand(const std::unordered_map<Engine::NodeId, NodePosition> &oldPositions,
        const std::unordered_map<Engine::NodeId, NodePosition> &newPositions,
        PositionSetter positionSetter)
        : oldPositions(oldPositions), newPositions(newPositions), positionSetter(std::move(positionSetter))
    {
    }

    void MoveNodesCommand::Execute()
    {
        for (const auto &[nodeId, pos] : newPositions)
        {
            positionSetter(nodeId, pos);
        }
    }

    void MoveNodesCommand::Undo()
    {
        for (const auto &[nodeId, pos] : oldPositions)
        {
            positionSetter(nodeId, pos);
        }
    }

    std::string MoveNodesCommand::GetDescription() const
    {
        if (newPositions.size() == 1)
        {
            return "Move Node";
        }
        return "Move " + std::to_string(newPositions.size()) + " Nodes";
    }

} // namespace VisionCraft
