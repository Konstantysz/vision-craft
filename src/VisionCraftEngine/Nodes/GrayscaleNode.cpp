#include "GrayscaleNode.h"
#include "Logger.h"

namespace VisionCraft::Engine
{
    GrayscaleNode::GrayscaleNode(NodeId id, const std::string& name)
        : Node(id, name)
    {
        SetParamValue("method", "BGR2GRAY");
        SetParamValue("preserveAlpha", "false");
    }

    void GrayscaleNode::Process()
    {
        if (inputImage.empty())
        {
            LOG_WARN("GrayscaleNode {}: No input image provided", GetName());
            outputImage = cv::Mat();
            return;
        }

        try
        {
            std::string methodStr = GetValidatedStringParam("method", "BGR2GRAY",
                {"BGR2GRAY", "RGB2GRAY", "BGRA2GRAY", "RGBA2GRAY"});
            bool preserveAlpha = GetBoolParam("preserveAlpha", false);

            if (inputImage.channels() == 1)
            {
                outputImage = inputImage.clone();
                LOG_INFO("GrayscaleNode {}: Input already grayscale, passing through", GetName());
                return;
            }

            int conversionCode = GetConversionMethod(methodStr);

            if (inputImage.channels() == 4 && preserveAlpha)
            {
                std::vector<cv::Mat> channels;
                cv::split(inputImage, channels);

                cv::Mat colorPart;
                if (inputImage.channels() == 4)
                {
                    cv::Mat rgb;
                    cv::merge(std::vector<cv::Mat>{channels[0], channels[1], channels[2]}, rgb);
                    cv::cvtColor(rgb, colorPart, conversionCode == cv::COLOR_BGRA2GRAY ? cv::COLOR_BGR2GRAY : cv::COLOR_RGB2GRAY);
                }

                std::vector<cv::Mat> grayAlpha = {colorPart, channels[3]};
                cv::merge(grayAlpha, outputImage);

                LOG_INFO("GrayscaleNode {}: Converted to grayscale with alpha preservation", GetName());
            }
            else
            {
                cv::cvtColor(inputImage, outputImage, conversionCode);
                LOG_INFO("GrayscaleNode {}: Converted to grayscale using method '{}'", GetName(), methodStr);
            }
        }
        catch (const cv::Exception& e)
        {
            LOG_ERROR("GrayscaleNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("GrayscaleNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
        }
    }

    int GrayscaleNode::GetConversionMethod(const std::string& methodStr) const
    {
        if (methodStr == "BGR2GRAY") return cv::COLOR_BGR2GRAY;
        if (methodStr == "RGB2GRAY") return cv::COLOR_RGB2GRAY;
        if (methodStr == "BGRA2GRAY") return cv::COLOR_BGRA2GRAY;
        if (methodStr == "RGBA2GRAY") return cv::COLOR_RGBA2GRAY;

        LOG_WARN("GrayscaleNode {}: Unknown conversion method '{}', using BGR2GRAY", GetName(), methodStr);
        return cv::COLOR_BGR2GRAY;
    }
} // namespace VisionCraft::Engine