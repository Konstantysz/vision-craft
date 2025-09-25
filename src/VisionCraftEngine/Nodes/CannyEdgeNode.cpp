#include "CannyEdgeNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Engine
{
    CannyEdgeNode::CannyEdgeNode(NodeId id, const std::string& name)
        : Node(id, name)
    {
        SetParam("lowThreshold", 50.0);
        SetParam("highThreshold", 150.0);
        SetParam("apertureSize", 3);
        SetParam("l2Gradient", false);
    }

    void CannyEdgeNode::Process()
    {
        if (inputImage.empty())
        {
            LOG_WARN("CannyEdgeNode {}: No input image provided", GetName());
            outputImage = cv::Mat();
            return;
        }

        try
        {
            const ValidationRange<double> kThresholdValidation{0.0};

            const auto lowThreshold = GetValidatedParam<double>("lowThreshold", 50.0, kThresholdValidation);
            const auto highThreshold = GetValidatedParam<double>("highThreshold", 150.0, kThresholdValidation);
            const auto apertureSize = GetParamOr<int>("apertureSize", 3);
            const auto l2Gradient = GetBoolParam("l2Gradient", false);

            if (apertureSize != 3 && apertureSize != 5 && apertureSize != 7) [[unlikely]]
            {
                LOG_WARN("CannyEdgeNode {}: Invalid aperture size ({}), using 3", GetName(), apertureSize);
                const_cast<int&>(apertureSize) = 3;
            }

            if (lowThreshold >= highThreshold)
            {
                LOG_WARN("CannyEdgeNode {}: Low threshold ({}) should be less than high threshold ({})",
                         GetName(), lowThreshold, highThreshold);
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

            LOG_INFO("CannyEdgeNode {}: Applied Canny edge detection (low: {}, high: {}, aperture: {}, l2: {})",
                     GetName(), lowThreshold, highThreshold, apertureSize, l2Gradient);
        }
        catch (const cv::Exception& e)
        {
            LOG_ERROR("CannyEdgeNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("CannyEdgeNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
        }
    }
} // namespace VisionCraft::Engine