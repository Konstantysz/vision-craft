#include "NodeFactory.h"
#include "Logger.h"
#include "Nodes/CannyEdgeNode.h"
#include "Nodes/GrayscaleNode.h"
#include "Nodes/ImageInputNode.h"
#include "Nodes/ImageOutputNode.h"
#include "Nodes/PreviewNode.h"
#include "Nodes/ThresholdNode.h"

namespace VisionCraft::Engine
{
    NodePtr NodeFactory::CreateNode(const std::string &type, NodeId id, const std::string &name)
    {
        if (type == "ImageInputNode")
        {
            return std::make_unique<ImageInputNode>(id, name);
        }
        if (type == "ImageOutputNode")
        {
            return std::make_unique<ImageOutputNode>(id, name);
        }
        if (type == "PreviewNode")
        {
            return std::make_unique<PreviewNode>(id, name);
        }
        if (type == "GrayscaleNode")
        {
            return std::make_unique<GrayscaleNode>(id, name);
        }
        if (type == "CannyEdgeNode")
        {
            return std::make_unique<CannyEdgeNode>(id, name);
        }
        if (type == "ThresholdNode")
        {
            return std::make_unique<ThresholdNode>(id, name);
        }

        LOG_ERROR("Unknown node type: {}", type);
        return nullptr;
    }
} // namespace VisionCraft::Engine
