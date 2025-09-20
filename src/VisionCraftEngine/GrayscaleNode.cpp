#include "GrayscaleNode.h"

#include "Logger.h"

#include <opencv2/imgproc.hpp>

namespace VisionCraft::Engine
{

    GrayscaleNode::GrayscaleNode(NodeId id) : Node(id, "Grayscale")
    {
        // No parameters for now
    }

    void GrayscaleNode::SetInputImage(const cv::Mat &img)
    {
        inputImage = img;
    }

    void GrayscaleNode::Process()
    {
        if (inputImage.empty())
        {
            LOG_WARN("[GrayscaleNode] No input image set!");
            return;
        }

        cv::cvtColor(inputImage, outputImage, cv::COLOR_BGR2GRAY);
    }
    
    const cv::Mat &GrayscaleNode::GetOutputImage() const
    {
        return outputImage;
    }
} // namespace VisionCraft::Engine
