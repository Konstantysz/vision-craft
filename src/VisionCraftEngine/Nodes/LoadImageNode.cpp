#include "LoadImageNode.h"
#include "Logger.h"

namespace VisionCraft::Engine
{
    LoadImageNode::LoadImageNode(NodeId id, const std::string& filePath)
        : Node(id, "Load Image")
    {
        SetParamValue("filePath", filePath);
    }

    void LoadImageNode::Process()
    {
        if (!ValidateFilePath("filePath", true))
        {
            image = cv::Mat();
            return;
        }

        const std::string filePath = GetParamValue("filePath").value();

        try
        {
            image = cv::imread(filePath, cv::IMREAD_UNCHANGED);

            if (image.empty())
            {
                LOG_ERROR("LoadImageNode {}: Failed to load image from '{}'", GetName(), filePath);
                return;
            }

            LOG_INFO("LoadImageNode {}: Successfully loaded image from '{}' ({}x{}, {} channels)",
                     GetName(), filePath, image.cols, image.rows, image.channels());
        }
        catch (const cv::Exception& e)
        {
            LOG_ERROR("LoadImageNode {}: OpenCV error loading image '{}': {}", GetName(), filePath, e.what());
            image = cv::Mat();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("LoadImageNode {}: Error loading image '{}': {}", GetName(), filePath, e.what());
            image = cv::Mat();
        }
    }
} // namespace VisionCraft::Engine