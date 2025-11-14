#include "Vision/Algorithms/CannyEdgeNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    CannyEdgeNode::CannyEdgeNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("LowThreshold", 50.0);
        CreateInputSlot("HighThreshold", 150.0);
        CreateInputSlot("ApertureSize", 3);
        CreateInputSlot("L2Gradient", false);
        CreateOutputSlot("Output");
    }

    void CannyEdgeNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("CannyEdgeNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            const auto lowThreshold = GetInputValue<double>("LowThreshold").value_or(50.0);
            const auto highThreshold = GetInputValue<double>("HighThreshold").value_or(150.0);
            const auto apertureSize = GetInputValue<int>("ApertureSize").value_or(3);
            const auto l2Gradient = GetInputValue<bool>("L2Gradient").value_or(false);

            if (apertureSize != 3 && apertureSize != 5 && apertureSize != 7) [[unlikely]]
            {
                LOG_WARN("CannyEdgeNode {}: Invalid aperture size ({}), using 3", GetName(), apertureSize);
                const_cast<int &>(apertureSize) = 3;
            }

            if (lowThreshold >= highThreshold)
            {
                LOG_WARN("CannyEdgeNode {}: Low threshold ({}) should be less than high threshold ({})",
                    GetName(),
                    lowThreshold,
                    highThreshold);
            }

            cv::Mat grayImage;
            if (inputImage.channels() > 1)
            {
                cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
            }
            else
            {
                grayImage = inputImage.clone();
            }

            cv::Canny(grayImage, outputImage, lowThreshold, highThreshold, apertureSize, l2Gradient);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("CannyEdgeNode {}: Applied Canny edge detection (low: {}, high: {}, aperture: {}, l2: {})",
                GetName(),
                lowThreshold,
                highThreshold,
                apertureSize,
                l2Gradient);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("CannyEdgeNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("CannyEdgeNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
