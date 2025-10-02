#include "Slot.h"

namespace VisionCraft::Engine
{
    void Slot::SetData(NodeData data)
    {
        this->data = std::move(data);
    }

    bool Slot::HasData() const
    {
        // std::monostate is index 0 - means empty/uninitialized
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

} // namespace VisionCraft::Engine
