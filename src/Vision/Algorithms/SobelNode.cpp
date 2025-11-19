#include "Vision/Algorithms/SobelNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    SobelNode::SobelNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("dx", 1);
        CreateInputSlot("dy", 1);
        CreateInputSlot("ksize", 3);
        CreateInputSlot("scale", 1.0);
        CreateInputSlot("delta", 0.0);
        CreateOutputSlot("Output");
    }

    void SobelNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("SobelNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            auto dx = GetInputValue<int>("dx").value_or(1);
            auto dy = GetInputValue<int>("dy").value_or(1);
            auto ksize = GetInputValue<int>("ksize").value_or(3);
            auto scale = GetInputValue<double>("scale").value_or(1.0);
            auto delta = GetInputValue<double>("delta").value_or(0.0);

            // Validate ksize (must be 1, 3, 5, or 7)
            if (ksize != 1 && ksize != 3 && ksize != 5 && ksize != 7) [[unlikely]]
            {
                LOG_WARN("SobelNode {}: Invalid ksize ({}), using 3", GetName(), ksize);
                ksize = 3;
            }

            cv::Mat grayImage;
            if (inputImage.channels() > 1)
            {
                cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
            }
            else
            {
                grayImage = inputImage;
            }

            // Use CV_16S to avoid overflow, then convert back to 8U
            cv::Mat grad;
            cv::Sobel(grayImage, grad, CV_16S, dx, dy, ksize, scale, delta, cv::BORDER_DEFAULT);
            cv::convertScaleAbs(grad, outputImage);

            SetOutputSlotData("Output", outputImage);

            LOG_INFO("SobelNode {}: Applied Sobel (dx: {}, dy: {}, ksize: {})", GetName(), dx, dy, ksize);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("SobelNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("SobelNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
