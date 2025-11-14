#include "Nodes/Factory/NodeFactory.h"

#include "Vision/Algorithms/CannyEdgeNode.h"
#include "Vision/Algorithms/GrayscaleNode.h"
#include "Vision/Algorithms/ThresholdNode.h"
#include "Vision/IO/ImageInputNode.h"
#include "Vision/IO/ImageOutputNode.h"
#include "Vision/IO/PreviewNode.h"

#include <algorithm>
#include <ranges>

namespace VisionCraft::Nodes
{
    std::unordered_map<std::string, NodeFactory::NodeCreator> &NodeFactory::GetRegistry()
    {
        static std::unordered_map<std::string, NodeCreator> registry;
        return registry;
    }

    void NodeFactory::Register(std::string_view type, NodeCreator creator)
    {
        GetRegistry()[std::string(type)] = std::move(creator);
    }

    std::unique_ptr<Node> NodeFactory::CreateNode(std::string_view type, NodeId id, std::string_view name)
    {
        auto &registry = GetRegistry();
        if (const auto it = registry.find(std::string(type)); it != registry.end())
        {
            return it->second(id, name);
        }
        return nullptr;
    }

    bool NodeFactory::IsRegistered(std::string_view type)
    {
        return GetRegistry().contains(std::string(type));
    }

    std::vector<std::string> NodeFactory::GetRegisteredTypes()
    {
        auto &registry = GetRegistry();
        std::vector<std::string> types;
        types.reserve(registry.size());

        for (const auto &[type, _] : registry)
        {
            types.push_back(type);
        }

        std::ranges::sort(types);
        return types;
    }

    void NodeFactory::RegisterAllNodes()
    {
        // Register all available node types with the factory
        Register("ImageInput", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::IO::ImageInputNode>(id, std::string(name));
        });

        Register("ImageOutput", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::IO::ImageOutputNode>(id, std::string(name));
        });

        Register("Preview", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::IO::PreviewNode>(id, std::string(name));
        });

        Register("Grayscale", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::Algorithms::GrayscaleNode>(id, std::string(name));
        });

        Register("CannyEdge", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::Algorithms::CannyEdgeNode>(id, std::string(name));
        });

        Register("Threshold", [](NodeId id, std::string_view name) {
            return std::make_unique<Vision::Algorithms::ThresholdNode>(id, std::string(name));
        });
    }
} // namespace VisionCraft::Nodes
