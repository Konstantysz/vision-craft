#include "Node.h"

namespace VisionCraft::Engine
{

    Node::Node(NodeId id, std::string name) : id(id), name(std::move(name))
    {
    }

    NodeId Node::GetId() const
    {
        return id;
    }

    const std::string &Node::GetName() const
    {
        return name;
    }

    const std::vector<NodeParam> &Node::GetParams() const
    {
        return params;
    }

    std::vector<NodeParam> &Node::GetParams()
    {
        return params;
    }

    std::optional<std::string> Node::GetParamValue(const std::string &paramName) const
    {
        for (const auto &param : params)
        {
            if (param.name == paramName)
            {
                return param.value;
            }
        }

        return std::nullopt;
    }

    void Node::SetParamValue(const std::string &paramName, const std::string &value)
    {
        for (auto &param : params)
        {
            if (param.name == paramName)
            {
                param.value = value;
                return;
            }
        }

        params.push_back({ paramName, value });
    }

} // namespace VisionCraft::Engine
