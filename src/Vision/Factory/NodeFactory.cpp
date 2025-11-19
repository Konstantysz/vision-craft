#include "Vision/Factory/NodeFactory.h"

#include "Vision/Algorithms/CannyEdgeNode.h"
#include "Vision/Algorithms/CvtColorNode.h"
#include "Vision/Algorithms/GrayscaleNode.h"
#include "Vision/Algorithms/MedianBlurNode.h"
#include "Vision/Algorithms/MergeChannelsNode.h"
#include "Vision/Algorithms/MorphologyNode.h"
#include "Vision/Algorithms/ResizeNode.h"
#include "Vision/Algorithms/SobelNode.h"
#include "Vision/Algorithms/SplitChannelsNode.h"
#include "Vision/Algorithms/ThresholdNode.h"
#include "Vision/IO/ImageInputNode.h"
#include "Vision/IO/ImageOutputNode.h"
#include "Vision/IO/PreviewNode.h"

#include <algorithm>
#include <ranges>

namespace VisionCraft::Vision
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

    std::unique_ptr<Nodes::Node> NodeFactory::CreateNode(std::string_view type, Nodes::NodeId id, std::string_view name)
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
        Register("ImageInput", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<IO::ImageInputNode>(id, std::string(name));
        });

        Register("ImageOutput", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<IO::ImageOutputNode>(id, std::string(name));
        });

        Register("Preview", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<IO::PreviewNode>(id, std::string(name));
        });

        Register("Grayscale", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::GrayscaleNode>(id, std::string(name));
        });

        Register("CannyEdge", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::CannyEdgeNode>(id, std::string(name));
        });

        Register("Threshold", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::ThresholdNode>(id, std::string(name));
        });

        Register("Sobel", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::SobelNode>(id, std::string(name));
        });

        Register("MedianBlur", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::MedianBlurNode>(id, std::string(name));
        });

        Register("Morphology", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::MorphologyNode>(id, std::string(name));
        });

        Register("CvtColor", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::CvtColorNode>(id, std::string(name));
        });

        Register("Resize", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::ResizeNode>(id, std::string(name));
        });

        Register("SplitChannels", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::SplitChannelsNode>(id, std::string(name));
        });

        Register("MergeChannels", [](Nodes::NodeId id, std::string_view name) {
            return std::make_unique<Algorithms::MergeChannelsNode>(id, std::string(name));
        });
    }
} // namespace VisionCraft::Vision
