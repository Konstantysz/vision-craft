#include "Vision/Algorithms/MorphologyNode.h"
#include "Logger.h"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace VisionCraft::Vision::Algorithms
{
    MorphologyNode::MorphologyNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        // Execution pins
        CreateExecutionInputPin("Execute");
        CreateExecutionOutputPin("Then");

        // Data pins
        CreateInputSlot("Input");
        CreateInputSlot("Operation", static_cast<int>(MorphOperation::Erode));
        CreateInputSlot("ksize", 3);
        CreateInputSlot("iterations", 1);
        CreateOutputSlot("Output");
    }

    void MorphologyNode::Process()
    {
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("MorphologyNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            auto op = GetInputValue<int>("Operation").value_or(static_cast<int>(MorphOperation::Erode));
            auto ksize = GetInputValue<int>("ksize").value_or(3);
            auto iterations = GetInputValue<int>("iterations").value_or(1);

            // Validate and clamp parameters
            ksize = std::max(1, ksize);
            iterations = std::max(1, iterations);

            // Map int to cv::MorphTypes using constexpr array
            constexpr std::array<cv::MorphTypes, 7> morphMapping{ cv::MORPH_ERODE,
                cv::MORPH_DILATE,
                cv::MORPH_OPEN,
                cv::MORPH_CLOSE,
                cv::MORPH_GRADIENT,
                cv::MORPH_TOPHAT,
                cv::MORPH_BLACKHAT };

            cv::MorphTypes morphOp;
            if (op >= 0 && op < static_cast<int>(morphMapping.size()))
            {
                morphOp = morphMapping[op];
            }
            else [[unlikely]]
            {
                LOG_WARN("MorphologyNode {}: Invalid operation ({}), using Erode", GetName(), op);
                morphOp = cv::MORPH_ERODE;
            }

            cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));

            cv::Mat outputImage;
            cv::morphologyEx(inputImage, outputImage, morphOp, element, cv::Point(-1, -1), iterations);
            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("MorphologyNode {}: Applied Morphology (Op: {}, ksize: {}, iter: {})",
                GetName(),
                op,
                ksize,
                iterations);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MorphologyNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MorphologyNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
