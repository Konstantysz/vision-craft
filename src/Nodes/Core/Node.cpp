#include "Nodes/Core/Node.h"

namespace VisionCraft::Nodes
{
    Node::Node(NodeId id, std::string name) : name(std::move(name)), id(id)
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

    float Node::CalculateExtraHeight([[maybe_unused]] float nodeContentWidth, [[maybe_unused]] float zoomLevel) const
    {
        return 0.0f;
    }

    Slot &Node::CreateInputSlot(const std::string &slotName)
    {
        return inputSlots[slotName];
    }

    Slot &Node::CreateOutputSlot(const std::string &slotName)
    {
        return outputSlots[slotName];
    }

    const Slot &Node::GetInputSlot(const std::string &slotName) const
    {
        return inputSlots.at(slotName);
    }

    const Slot &Node::GetOutputSlot(const std::string &slotName) const
    {
        return outputSlots.at(slotName);
    }

    void Node::SetInputSlotData(const std::string &slotName, NodeData data)
    {
        inputSlots.at(slotName).SetData(std::move(data));
    }

    void Node::SetOutputSlotData(const std::string &slotName, NodeData data)
    {
        outputSlots.at(slotName).SetData(std::move(data));
    }

    void Node::ClearInputSlot(const std::string &slotName)
    {
        inputSlots.at(slotName).Clear();
    }

    void Node::ClearOutputSlot(const std::string &slotName)
    {
        outputSlots.at(slotName).Clear();
    }

    void Node::SetInputSlotDefault(const std::string &slotName, NodeData defaultValue)
    {
        inputSlots.at(slotName).SetDefaultValue(std::move(defaultValue));
    }

    bool Node::IsInputSlotConnected(const std::string &slotName) const
    {
        return inputSlots.at(slotName).IsConnected();
    }

    bool Node::HasInputSlot(const std::string &slotName) const
    {
        return inputSlots.find(slotName) != inputSlots.end();
    }

    bool Node::HasOutputSlot(const std::string &slotName) const
    {
        return outputSlots.find(slotName) != outputSlots.end();
    }

    void Node::CreateExecutionInputPin(const std::string &pinName)
    {
        executionInputPins.insert(pinName);
    }

    void Node::CreateExecutionOutputPin(const std::string &pinName)
    {
        executionOutputPins.insert(pinName);
    }

    bool Node::HasExecutionInputPin(const std::string &pinName) const
    {
        return executionInputPins.count(pinName) > 0;
    }

    bool Node::HasExecutionOutputPin(const std::string &pinName) const
    {
        return executionOutputPins.count(pinName) > 0;
    }

    std::vector<std::string> Node::GetExecutionInputPins() const
    {
        return std::vector<std::string>(executionInputPins.begin(), executionInputPins.end());
    }

    std::vector<std::string> Node::GetExecutionOutputPins() const
    {
        return std::vector<std::string>(executionOutputPins.begin(), executionOutputPins.end());
    }

    template<typename T> Slot &Node::CreateInputSlot(const std::string &slotName, T defaultValue)
    {
        NodeData nodeData = std::move(defaultValue);
        inputSlots[slotName] = Slot(std::optional<NodeData>(std::move(nodeData)));
        return inputSlots[slotName];
    }
    template Slot &Node::CreateInputSlot<double>(const std::string &, double);
    template Slot &Node::CreateInputSlot<float>(const std::string &, float);
    template Slot &Node::CreateInputSlot<int>(const std::string &, int);
    template Slot &Node::CreateInputSlot<bool>(const std::string &, bool);
    template Slot &Node::CreateInputSlot<std::string>(const std::string &, std::string);
    template Slot &Node::CreateInputSlot<std::filesystem::path>(const std::string &, std::filesystem::path);

} // namespace VisionCraft::Nodes
