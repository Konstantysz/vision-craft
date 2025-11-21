#include "Vision/Algorithms/SobelNode.h"
#include "Logger.h"
#include <algorithm>
#include <array>
#include <ranges>
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    SobelNode::SobelNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        // Execution pins
        CreateExecutionInputPin("Execute");
        CreateExecutionOutputPin("Then");

        // Data pins
        CreateInputSlot("Input");
        CreateInputSlot("dx", 1);
        CreateInputSlot("dy", 1);
        CreateInputSlot("ksize", 3);
        CreateInputSlot("scale", 1.0);
        CreateInputSlot("delta", 0.0);
        CreateOutputSlot("Output");
    }

    void SobelNode::Process()
    {
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("SobelNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            auto dx = GetInputValue<int>("dx").value_or(1);
            auto dy = GetInputValue<int>("dy").value_or(1);
            auto ksize = GetInputValue<int>("ksize").value_or(3);
            auto scale = GetInputValue<double>("scale").value_or(1.0);
            auto delta = GetInputValue<double>("delta").value_or(0.0);

            // Validate dx and dy (must satisfy: dx + dy > 0 and dx + dy <= 2)
            if (dx < 0 || dy < 0 || (dx + dy) == 0 || (dx + dy) > 2) [[unlikely]]
            {
                LOG_WARN("SobelNode {}: Invalid dx ({}) or dy ({}), using dx=1, dy=1", GetName(), dx, dy);
                dx = 1;
                dy = 1;
            }

            // Validate ksize (must be 1, 3, 5, or 7)
            constexpr std::array validKsizes{ 1, 3, 5, 7 };
            if (std::ranges::find(validKsizes, ksize) == validKsizes.end()) [[unlikely]]
            {
                LOG_WARN("SobelNode {}: Invalid ksize ({}), using 3", GetName(), ksize);
                ksize = 3;
            }

            const cv::Mat grayImage = [&]() {
                if (inputImage.channels() > 1)
                {
                    cv::Mat result;
                    cv::cvtColor(inputImage, result, cv::COLOR_BGR2GRAY);
                    return result;
                }
                return inputImage;
            }();

            // Use CV_16S to avoid overflow, then convert back to 8U
            cv::Mat grad;
            cv::Sobel(grayImage, grad, CV_16S, dx, dy, ksize, scale, delta, cv::BORDER_DEFAULT);

            cv::Mat outputImage;
            cv::convertScaleAbs(grad, outputImage);

            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("SobelNode {}: Applied Sobel (dx: {}, dy: {}, ksize: {})", GetName(), dx, dy, ksize);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("SobelNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("SobelNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
