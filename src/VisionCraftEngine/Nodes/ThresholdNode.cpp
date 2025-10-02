#include "ThresholdNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Engine
{
    ThresholdNode::ThresholdNode(NodeId id, const std::string &name) : Node(id, name)
    {
        // Create input and output slots
        CreateInputSlot("Input");
        CreateOutputSlot("Output");

        SetParam("threshold", 127.0);
        SetParam("maxValue", 255.0);
        SetParam("type", std::string{ "THRESH_BINARY" });
    }

    void ThresholdNode::Process()
    {
        // Get input image from slot
        auto inputData = GetInputSlot("Input").GetData<cv::Mat>();
        if (!inputData || inputData->empty())
        {
            LOG_WARN("ThresholdNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        // Store input
        inputImage = *inputData;

        try
        {
            const ValidationRange<double> kThresholdRange{ 0.0, 255.0 };
            const StringValidation kTypeValidation{ { "THRESH_BINARY",
                                                        "THRESH_BINARY_INV",
                                                        "THRESH_TRUNC",
                                                        "THRESH_TOZERO",
                                                        "THRESH_TOZERO_INV",
                                                        "THRESH_OTSU",
                                                        "THRESH_TRIANGLE" },
                true };

            const auto threshold = GetValidatedParam<double>("threshold", 127.0, kThresholdRange);
            const auto maxValue = GetValidatedParam<double>("maxValue", 255.0, kThresholdRange);
            const auto typeStr = GetValidatedString("type", "THRESH_BINARY", kTypeValidation);

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

            // Write to output slot
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