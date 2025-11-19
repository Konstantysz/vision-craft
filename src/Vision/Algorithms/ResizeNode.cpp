#include "Vision/Algorithms/ResizeNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    ResizeNode::ResizeNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("Width", 0);  // 0 means use scale
        CreateInputSlot("Height", 0); // 0 means use scale
        CreateInputSlot("ScaleX", 1.0);
        CreateInputSlot("ScaleY", 1.0);
        // 0: Linear, 1: Nearest, 2: Cubic, 3: Area, 4: Lanczos4
        CreateInputSlot("Interpolation", 0);
        CreateOutputSlot("Output");
    }

    void ResizeNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("ResizeNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;

        try
        {
            auto width = GetInputValue<int>("Width").value_or(0);
            auto height = GetInputValue<int>("Height").value_or(0);
            auto scaleX = GetInputValue<double>("ScaleX").value_or(1.0);
            auto scaleY = GetInputValue<double>("ScaleY").value_or(1.0);
            auto interp = GetInputValue<int>("Interpolation").value_or(0);

            cv::InterpolationFlags flags;
            switch (interp)
            {
            case 0:
                flags = cv::INTER_LINEAR;
                break;
            case 1:
                flags = cv::INTER_NEAREST;
                break;
            case 2:
                flags = cv::INTER_CUBIC;
                break;
            case 3:
                flags = cv::INTER_AREA;
                break;
            case 4:
                flags = cv::INTER_LANCZOS4;
                break;
            default:
                flags = cv::INTER_LINEAR;
                break;
            }

            cv::Size dsize(width, height);
            double fx = scaleX;
            double fy = scaleY;

            // If width/height are specified, they take precedence over scale
            if (width > 0 && height > 0)
            {
                fx = 0;
                fy = 0;
            }
            else
            {
                // Ensure scale is valid
                if (fx <= 0)
                    fx = 1.0;
                if (fy <= 0)
                    fy = 1.0;
                dsize = cv::Size(0, 0);
            }

            cv::resize(inputImage, outputImage, dsize, fx, fy, flags);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("ResizeNode {}: Resized (Size: {}x{}, Scale: {}x{}, Interp: {})",
                GetName(),
                width,
                height,
                fx,
                fy,
                interp);
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ResizeNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ResizeNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
