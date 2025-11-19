#include "Vision/Algorithms/CvtColorNode.h"
#include "Logger.h"
#include <array>
#include <stdexcept>
#include <utility>

namespace VisionCraft::Vision::Algorithms
{
    CvtColorNode::CvtColorNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("Conversion", static_cast<int>(ColorConversion::BGR2GRAY));
        CreateOutputSlot("Output");
    }

    void CvtColorNode::Process()
    {
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("CvtColorNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            auto conversion = GetInputValue<int>("Conversion").value_or(static_cast<int>(ColorConversion::BGR2GRAY));

            // Define conversion requirements using constexpr array
            constexpr std::array<ConversionInfo, 6> conversions{ ConversionInfo{ cv::COLOR_BGR2GRAY, 3, "BGR2GRAY" },
                ConversionInfo{ cv::COLOR_GRAY2BGR, 1, "GRAY2BGR" },
                ConversionInfo{ cv::COLOR_BGR2RGB, 3, "BGR2RGB" },
                ConversionInfo{ cv::COLOR_RGB2BGR, 3, "RGB2BGR" },
                ConversionInfo{ cv::COLOR_BGR2HSV, 3, "BGR2HSV" },
                ConversionInfo{ cv::COLOR_HSV2BGR, 3, "HSV2BGR" } };

            if (conversion < 0 || conversion >= static_cast<int>(conversions.size())) [[unlikely]]
            {
                LOG_WARN("CvtColorNode {}: Invalid conversion code ({}), using BGR2GRAY", GetName(), conversion);
                conversion = static_cast<int>(ColorConversion::BGR2GRAY);
            }

            const auto &convInfo = conversions[conversion];

            // Validate channel count
            if (inputImage.channels() != convInfo.requiredChannels) [[unlikely]]
            {
                LOG_ERROR("CvtColorNode {}: {} requires {}-channel input, got {}",
                    GetName(),
                    convInfo.name,
                    convInfo.requiredChannels,
                    inputImage.channels());
                ClearOutputSlot("Output");
                return;
            }

            cv::Mat outputImage;
            cv::cvtColor(inputImage, outputImage, convInfo.code);
            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("CvtColorNode {}: Applied Color Conversion ({})", GetName(), convInfo.name);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("CvtColorNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("CvtColorNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
