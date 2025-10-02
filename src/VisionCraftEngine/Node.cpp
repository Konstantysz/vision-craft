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

    float Node::CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const
    {
        // Default implementation: no extra height needed
        // Derived classes can override this for custom content
        return 0.0f;
    }

    Slot &Node::CreateInputSlot(const std::string &slotName)
    {
        return inputSlots[slotName]; // Creates if doesn't exist
    }

    Slot &Node::CreateOutputSlot(const std::string &slotName)
    {
        return outputSlots[slotName]; // Creates if doesn't exist
    }

    const Slot &Node::GetInputSlot(const std::string &slotName) const
    {
        return inputSlots.at(slotName); // Throws if doesn't exist
    }

    const Slot &Node::GetOutputSlot(const std::string &slotName) const
    {
        return outputSlots.at(slotName); // Throws if doesn't exist
    }

    void Node::SetInputSlotData(const std::string &slotName, NodeData data)
    {
        inputSlots.at(slotName).SetData(std::move(data)); // Throws if slot doesn't exist
    }

    void Node::SetOutputSlotData(const std::string &slotName, NodeData data)
    {
        outputSlots.at(slotName).SetData(std::move(data)); // Throws if slot doesn't exist
    }

    void Node::ClearInputSlot(const std::string &slotName)
    {
        inputSlots.at(slotName).Clear(); // Throws if slot doesn't exist
    }

    void Node::ClearOutputSlot(const std::string &slotName)
    {
        outputSlots.at(slotName).Clear(); // Throws if slot doesn't exist
    }

    void Node::SetInputSlotDefault(const std::string &slotName, NodeData defaultValue)
    {
        inputSlots.at(slotName).SetDefaultValue(std::move(defaultValue)); // Throws if slot doesn't exist
    }

    bool Node::IsInputSlotConnected(const std::string &slotName) const
    {
        return inputSlots.at(slotName).IsConnected(); // Throws if slot doesn't exist
    }

    bool Node::HasInputSlot(const std::string &slotName) const
    {
        return inputSlots.find(slotName) != inputSlots.end();
    }

    bool Node::HasOutputSlot(const std::string &slotName) const
    {
        return outputSlots.find(slotName) != outputSlots.end();
    }

    // Template implementation for CreateInputSlot
    template<typename T> Slot &Node::CreateInputSlot(const std::string &slotName, T defaultValue)
    {
        NodeData nodeData = std::move(defaultValue);
        inputSlots[slotName] = Slot(std::optional<NodeData>(std::move(nodeData)));
        return inputSlots[slotName];
    }

    // Explicit instantiations
    template Slot &Node::CreateInputSlot<double>(const std::string &, double);
    template Slot &Node::CreateInputSlot<float>(const std::string &, float);
    template Slot &Node::CreateInputSlot<int>(const std::string &, int);
    template Slot &Node::CreateInputSlot<bool>(const std::string &, bool);
    template Slot &Node::CreateInputSlot<std::string>(const std::string &, std::string);
    template Slot &Node::CreateInputSlot<std::filesystem::path>(const std::string &, std::filesystem::path);

} // namespace VisionCraft::Engine