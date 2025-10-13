#include "ThresholdNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Engine
{
    ThresholdNode::ThresholdNode(NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("Threshold", 127.0);
        CreateInputSlot("MaxValue", 255.0);
        CreateInputSlot("Type", std::string{ "THRESH_BINARY" });
        CreateOutputSlot("Output");
    }

    void ThresholdNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("ThresholdNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            const auto threshold = GetInputValue<double>("Threshold").value_or(127.0);
            const auto maxValue = GetInputValue<double>("MaxValue").value_or(255.0);
            const auto typeStr = GetInputValue<std::string>("Type").value_or("THRESH_BINARY");
            int thresholdType = GetThresholdType(typeStr);

            cv::Mat grayImage;
            if (inputImage.channels() > 1)
            {
                cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
            }
            else
            {
                grayImage = inputImage.clone();
            }

            double actualThreshold = cv::threshold(grayImage, outputImage, threshold, maxValue, thresholdType);

            SetOutputSlotData("Output", outputImage);

            LOG_INFO("ThresholdNode {}: Applied thresholding (threshold: {}, actual: {}, max: {}, type: {})",
                GetName(),
                threshold,
                actualThreshold,
                maxValue,
                typeStr);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ThresholdNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ThresholdNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }

    int ThresholdNode::GetThresholdType(const std::string &typeStr) const
    {
        if (typeStr == "THRESH_BINARY")
            return cv::THRESH_BINARY;
        if (typeStr == "THRESH_BINARY_INV")
            return cv::THRESH_BINARY_INV;
        if (typeStr == "THRESH_TRUNC")
            return cv::THRESH_TRUNC;
        if (typeStr == "THRESH_TOZERO")
            return cv::THRESH_TOZERO;
        if (typeStr == "THRESH_TOZERO_INV")
            return cv::THRESH_TOZERO_INV;
        if (typeStr == "THRESH_OTSU")
            return cv::THRESH_OTSU;
        if (typeStr == "THRESH_TRIANGLE")
            return cv::THRESH_TRIANGLE;

        LOG_WARN("ThresholdNode {}: Unknown threshold type '{}', using THRESH_BINARY", GetName(), typeStr);
        return cv::THRESH_BINARY;
    }
} // namespace VisionCraft::Engine
