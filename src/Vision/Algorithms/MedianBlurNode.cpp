#include "Vision/Algorithms/MedianBlurNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    MedianBlurNode::MedianBlurNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("ksize", 3);
        CreateOutputSlot("Output");
    }

    void MedianBlurNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("MedianBlurNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            auto ksize = GetInputValue<int>("ksize").value_or(3);

            // Validate ksize (must be odd and greater than 1)
            if (ksize % 2 == 0)
            {
                ksize++;
                LOG_WARN("MedianBlurNode {}: ksize must be odd, adjusting to {}", GetName(), ksize);
            }
            if (ksize < 1)
            {
                ksize = 1;
                LOG_WARN("MedianBlurNode {}: ksize must be >= 1, adjusting to {}", GetName(), ksize);
            }

            cv::medianBlur(inputImage, outputImage, ksize);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("MedianBlurNode {}: Applied Median Blur (ksize: {})", GetName(), ksize);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MedianBlurNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MedianBlurNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
