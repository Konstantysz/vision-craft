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
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("MedianBlurNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            auto ksize = GetInputValue<int>("ksize").value_or(3);

            // Validate ksize (must be odd and >= 3)
            if (ksize < 3) [[unlikely]]
            {
                ksize = 3;
                LOG_WARN("MedianBlurNode {}: ksize must be >= 3, adjusting to {}", GetName(), ksize);
            }
            if (ksize % 2 == 0) [[unlikely]]
            {
                ksize++;
                LOG_WARN("MedianBlurNode {}: ksize must be odd, adjusting to {}", GetName(), ksize);
            }

            cv::Mat outputImage;
            cv::medianBlur(inputImage, outputImage, ksize);
            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("MedianBlurNode {}: Applied Median Blur (ksize: {})", GetName(), ksize);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MedianBlurNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MedianBlurNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
