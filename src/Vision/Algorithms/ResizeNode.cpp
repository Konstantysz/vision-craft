#include "Vision/Algorithms/ResizeNode.h"
#include "Logger.h"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace VisionCraft::Vision::Algorithms
{
    ResizeNode::ResizeNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        // Execution pins
        CreateExecutionInputPin("Execute");
        CreateExecutionOutputPin("Then");

        // Data pins
        CreateInputSlot("Input");
        CreateInputSlot("Width", 0);  // 0 means use scale
        CreateInputSlot("Height", 0); // 0 means use scale
        CreateInputSlot("ScaleX", 1.0);
        CreateInputSlot("ScaleY", 1.0);
        CreateInputSlot("Interpolation", static_cast<int>(InterpolationMethod::Linear));
        CreateOutputSlot("Output");
    }

    void ResizeNode::Process()
    {
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("ResizeNode {}: No input image provided", GetName());
            ClearOutputSlot("Output");
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            auto width = GetInputValue<int>("Width").value_or(0);
            auto height = GetInputValue<int>("Height").value_or(0);
            auto scaleX = GetInputValue<double>("ScaleX").value_or(1.0);
            auto scaleY = GetInputValue<double>("ScaleY").value_or(1.0);
            auto interp = GetInputValue<int>("Interpolation").value_or(static_cast<int>(InterpolationMethod::Linear));

            // Map interpolation using constexpr array
            constexpr std::array<cv::InterpolationFlags, 5> interpMapping{
                cv::INTER_LINEAR, cv::INTER_NEAREST, cv::INTER_CUBIC, cv::INTER_AREA, cv::INTER_LANCZOS4
            };

            cv::InterpolationFlags flags;
            if (interp >= 0 && interp < static_cast<int>(interpMapping.size()))
            {
                flags = interpMapping[interp];
            }
            else [[unlikely]]
            {
                flags = cv::INTER_LINEAR;
            }

            cv::Size dsize{ 0, 0 };
            double fx = scaleX;
            double fy = scaleY;

            // If width/height are specified, they take precedence over scale
            if (width > 0 && height > 0)
            {
                dsize = cv::Size{ width, height };
                fx = fy = 0;
            }
            else if (width > 0 || height > 0) [[unlikely]]
            {
                LOG_WARN("ResizeNode {}: Both width and height must be specified or both zero, using scale", GetName());
                fx = std::clamp(fx, 0.01, 100.0);
                fy = std::clamp(fy, 0.01, 100.0);
            }
            else
            {
                // Use scale factors with clamping
                fx = std::clamp(fx, 0.01, 100.0);
                fy = std::clamp(fy, 0.01, 100.0);
            }

            cv::Mat outputImage;
            cv::resize(inputImage, outputImage, dsize, fx, fy, flags);
            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("ResizeNode {}: Resized (Size: {}x{}, Scale: {:.2f}x{:.2f}, Interp: {})",
                GetName(),
                outputImage.cols,
                outputImage.rows,
                fx,
                fy,
                interp);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ResizeNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ResizeNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
