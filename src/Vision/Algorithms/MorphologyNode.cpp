#include "Vision/Algorithms/MorphologyNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    MorphologyNode::MorphologyNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("Operation", 0); // 0: Erode, 1: Dilate, 2: Open, 3: Close, 4: Gradient, 5: TopHat, 6: BlackHat
        CreateInputSlot("ksize", 3);
        CreateInputSlot("iterations", 1);
        CreateOutputSlot("Output");
    }

    void MorphologyNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("MorphologyNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            auto op = GetInputValue<int>("Operation").value_or(0);
            auto ksize = GetInputValue<int>("ksize").value_or(3);
            auto iterations = GetInputValue<int>("iterations").value_or(1);

            // Map int to cv::MorphTypes
            cv::MorphTypes morphOp;
            switch (op)
            {
            case 0:
                morphOp = cv::MORPH_ERODE;
                break;
            case 1:
                morphOp = cv::MORPH_DILATE;
                break;
            case 2:
                morphOp = cv::MORPH_OPEN;
                break;
            case 3:
                morphOp = cv::MORPH_CLOSE;
                break;
            case 4:
                morphOp = cv::MORPH_GRADIENT;
                break;
            case 5:
                morphOp = cv::MORPH_TOPHAT;
                break;
            case 6:
                morphOp = cv::MORPH_BLACKHAT;
                break;
            default:
                LOG_WARN("MorphologyNode {}: Invalid operation ({}), using Erode", GetName(), op);
                morphOp = cv::MORPH_ERODE;
                break;
            }

            if (ksize < 1)
                ksize = 1;
            cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));

            cv::morphologyEx(inputImage, outputImage, morphOp, element, cv::Point(-1, -1), iterations);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("MorphologyNode {}: Applied Morphology (Op: {}, ksize: {}, iter: {})",
                GetName(),
                op,
                ksize,
                iterations);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MorphologyNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MorphologyNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
