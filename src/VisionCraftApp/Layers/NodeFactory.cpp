#include "NodeFactory.h"

#include <algorithm>
#include <ranges>

namespace VisionCraft
{
    void NodeFactory::Register(std::string_view type, NodeCreator creator)
    {
        registry[std::string(type)] = std::move(creator);
    }

    std::unique_ptr<Engine::Node>
        NodeFactory::Create(std::string_view type, Engine::NodeId id, std::string_view name) const
    {
        if (const auto it = registry.find(std::string(type)); it != registry.end())
        {
            return it->second(id, name);
        }
        return nullptr;
    }

    bool NodeFactory::IsRegistered(std::string_view type) const
    {
        return registry.contains(std::string(type));
    }

    std::vector<std::string> NodeFactory::GetRegisteredTypes() const
    {
        std::vector<std::string> types;
        types.reserve(registry.size());

        for (const auto &[type, _] : registry)
        {
            types.push_back(type);
        }

        std::ranges::sort(types);
        return types;
    }
} // namespace VisionCraft
