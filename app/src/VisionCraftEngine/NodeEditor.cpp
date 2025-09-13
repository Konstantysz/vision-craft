#include "NodeEditor.h"
#include <algorithm>

namespace vc
{

    NodeEditor::NodeEditor() : nextId(1)
    {
    }

    NodeId NodeEditor::AddNode(NodePtr node)
    {
        NodeId id = node->GetId();
        if (id == 0)
        {
            id = nextId++;
            node->SetId(id);
        }
        else if (id >= nextId)
        {
            nextId = id + 1;
        }
        nodes[id] = std::move(node);
        return id;
    }

    bool NodeEditor::RemoveNode(NodeId id)
    {
        auto it = nodes.find(id);
        if (it == nodes.end())
        {
            return false;
        }
        nodes.erase(it);
        connections.erase(std::remove_if(connections.begin(),
                              connections.end(),
                              [id](const Connection &c) { return c.from == id || c.to == id; }),
            connections.end());
        return true;
    }

    Node *NodeEditor::GetNode(NodeId id)
    {
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    const Node *NodeEditor::GetNode(NodeId id) const
    {
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    std::vector<NodeId> NodeEditor::GetNodeIds() const
    {
        std::vector<NodeId> ids;
        ids.reserve(nodes.size());
        for (const auto &[id, _] : nodes)
        {
            ids.push_back(id);
        }
        return ids;
    }

    void NodeEditor::AddConnection(NodeId from, NodeId to)
    {
        connections.push_back({ from, to });
    }

    bool NodeEditor::RemoveConnection(NodeId from, NodeId to)
    {
        auto it = std::remove_if(connections.begin(), connections.end(), [from, to](const Connection &c) {
            return c.from == from && c.to == to;
        });

        if (it == connections.end())
        {
            return false;
        }

        connections.erase(it, connections.end());
        
        return true;
    }

    const std::vector<Connection> &NodeEditor::GetConnections() const
    {
        return connections;
    }

    void NodeEditor::Clear()
    {
        nodes.clear();
        connections.clear();
        nextId = 1;
    }

} // namespace vc
