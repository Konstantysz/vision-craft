#include "ImageInputNode.h"
#include "Logger.h"

namespace VisionCraft::Engine
{
    ImageInputNode::ImageInputNode(NodeId id, const std::string& name)
        : Node(id, name)
    {
        // Set default parameters
        SetParamValue("filepath", "");
    }

    void ImageInputNode::Process()
    {
        if (!ValidateFilePath("filepath", true))
        {
            outputImage = cv::Mat();
            return;
        }

        const std::string filepath = GetParamValue("filepath").value();

        try
        {
            outputImage = cv::imread(filepath, cv::IMREAD_COLOR);
            if (outputImage.empty())
            {
                LOG_ERROR("ImageInputNode {}: Failed to load image from '{}'", GetName(), filepath);
                return;
            }

            LOG_INFO("ImageInputNode {}: Successfully loaded image from '{}' ({}x{}, {} channels)",
                     GetName(), filepath, outputImage.cols, outputImage.rows, outputImage.channels());
        }
        catch (const cv::Exception& e)
        {
            LOG_ERROR("ImageInputNode {}: OpenCV error loading image '{}': {}", GetName(), filepath, e.what());
            outputImage = cv::Mat();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ImageInputNode {}: Error loading image '{}': {}", GetName(), filepath, e.what());
            outputImage = cv::Mat();
        }
    }
} // namespace VisionCraft::Engine