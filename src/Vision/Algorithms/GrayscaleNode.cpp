#include "Vision/Algorithms/GrayscaleNode.h"
#include "Logger.h"

namespace VisionCraft::Vision::Algorithms
{
    GrayscaleNode::GrayscaleNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("Method", std::string{ "BGR2GRAY" });
        CreateInputSlot("PreserveAlpha", false);
        CreateOutputSlot("Output");
    }

    void GrayscaleNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("GrayscaleNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            const auto methodStr = GetInputValue<std::string>("Method").value_or("BGR2GRAY");
            const auto preserveAlpha = GetInputValue<bool>("PreserveAlpha").value_or(false);

            cv::Mat outputImage;

            if (inputImage.channels() == 1)
            {
                outputImage = inputImage.clone();
                LOG_INFO("GrayscaleNode {}: Input already grayscale, passing through", GetName());
            }
            else
            {
                int conversionCode = GetConversionMethod(methodStr);

                if (inputImage.channels() == 4 && preserveAlpha)
                {
                    std::vector<cv::Mat> channels;
                    cv::split(inputImage, channels);

                    cv::Mat colorPart;
                    if (inputImage.channels() == 4)
                    {
                        cv::Mat rgb;
                        cv::merge(std::vector<cv::Mat>{ channels[0], channels[1], channels[2] }, rgb);
                        cv::cvtColor(rgb,
                            colorPart,
                            conversionCode == cv::COLOR_BGRA2GRAY ? cv::COLOR_BGR2GRAY : cv::COLOR_RGB2GRAY);
                    }

                    std::vector<cv::Mat> grayAlpha = { colorPart, channels[3] };
                    cv::merge(grayAlpha, outputImage);

                    LOG_INFO("GrayscaleNode {}: Converted to grayscale with alpha preservation", GetName());
                }
                else
                {
                    cv::cvtColor(inputImage, outputImage, conversionCode);
                    LOG_INFO("GrayscaleNode {}: Converted to grayscale using method '{}'", GetName(), methodStr);
                }
            }

            SetOutputSlotData("Output", outputImage);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("GrayscaleNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("GrayscaleNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }

    int GrayscaleNode::GetConversionMethod(const std::string &methodStr) const
    {
        if (methodStr == "BGR2GRAY")
            return cv::COLOR_BGR2GRAY;
        if (methodStr == "RGB2GRAY")
            return cv::COLOR_RGB2GRAY;
        if (methodStr == "BGRA2GRAY")
            return cv::COLOR_BGRA2GRAY;
        if (methodStr == "RGBA2GRAY")
            return cv::COLOR_RGBA2GRAY;

        LOG_WARN("GrayscaleNode {}: Unknown conversion method '{}', using BGR2GRAY", GetName(), methodStr);
        return cv::COLOR_BGR2GRAY;
    }
} // namespace VisionCraft::Vision::Algorithms
