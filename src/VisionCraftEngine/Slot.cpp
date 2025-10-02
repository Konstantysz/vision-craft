#include "Slot.h"

namespace VisionCraft::Engine
{
    Slot::Slot(std::optional<NodeData> defaultValue) : defaultValue(std::move(defaultValue))
    {
    }

    void Slot::SetData(NodeData data)
    {
        this->data = std::move(data);
    }

    bool Slot::HasData() const
    {
        return data.index() != 0;
    }

    void Slot::Clear()
    {
        data = std::monostate{};
    }

    size_t Slot::GetTypeIndex() const
    {
        return data.index();
    }

    void Slot::SetDefaultValue(NodeData defaultValue)
    {
        this->defaultValue = std::move(defaultValue);
    }

    bool Slot::HasDefaultValue() const
    {
        return defaultValue.has_value();
    }

    bool Slot::IsConnected() const
    {
        return HasData();
    }

    const NodeData &Slot::GetVariantData() const
    {
        return data;
    }

} // namespace VisionCraft::Engine
