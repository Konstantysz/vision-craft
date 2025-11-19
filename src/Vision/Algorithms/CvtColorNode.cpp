#include "Vision/Algorithms/CvtColorNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    CvtColorNode::CvtColorNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        // 0: BGR2GRAY, 1: GRAY2BGR, 2: BGR2RGB, 3: RGB2BGR, 4: BGR2HSV, 5: HSV2BGR
        CreateInputSlot("Conversion", 0);
        CreateOutputSlot("Output");
    }

    void CvtColorNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("CvtColorNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            auto conversion = GetInputValue<int>("Conversion").value_or(0);
            cv::ColorConversionCodes code;

            switch (conversion)
            {
            case 0:
                code = cv::COLOR_BGR2GRAY;
                break;
            case 1:
                code = cv::COLOR_GRAY2BGR;
                break;
            case 2:
                code = cv::COLOR_BGR2RGB;
                break;
            case 3:
                code = cv::COLOR_RGB2BGR;
                break;
            case 4:
                code = cv::COLOR_BGR2HSV;
                break;
            case 5:
                code = cv::COLOR_HSV2BGR;
                break;
            default:
                LOG_WARN("CvtColorNode {}: Invalid conversion code ({}), using BGR2GRAY", GetName(), conversion);
                code = cv::COLOR_BGR2GRAY;
                break;
            }

            // Check channel compatibility for some conversions if needed, but OpenCV handles most errors
            cv::cvtColor(inputImage, outputImage, code);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("CvtColorNode {}: Applied Color Conversion (Code: {})", GetName(), conversion);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("CvtColorNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("CvtColorNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
