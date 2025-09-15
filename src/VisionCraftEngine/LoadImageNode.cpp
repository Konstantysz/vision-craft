#include "LoadImageNode.h"

#include "Logger.h"

#include <opencv2/imgcodecs.hpp>

namespace vc
{

    LoadImageNode::LoadImageNode(NodeId id, const std::string &filePath) : Node(id, "Load Image")
    {
        params.push_back({ "filePath", filePath });
    }

    void LoadImageNode::Process()
    {
        auto filePath = GetParamValue("filePath");
        if (!filePath)
        {
            LOG_WARN("[LoadImageNode] No file path set!");
            return;
        }

        image = cv::imread(*filePath, cv::IMREAD_UNCHANGED);
        if (!image.empty())
        {
            LOG_ERROR("[LoadImageNode] Failed to load image: {}", *filePath);
            return;
        }

        LOG_INFO("[LoadImageNode] Loaded image: {} ({}x{})", *filePath, image.cols, image.rows);
    }

} // namespace vc
