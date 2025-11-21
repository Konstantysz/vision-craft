#include "Vision/Algorithms/SplitChannelsNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    SplitChannelsNode::SplitChannelsNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        // Execution pins
        CreateExecutionInputPin("Execute");
        CreateExecutionOutputPin("Then");

        // Data pins
        CreateInputSlot("Input");
        for (const auto &slotName : kChannelSlots)
        {
            CreateOutputSlot(slotName);
        }
    }

    void SplitChannelsNode::Process()
    {
        const auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty()) [[unlikely]]
        {
            LOG_WARN("SplitChannelsNode {}: No input image provided", GetName());
            for (const auto &slotName : kChannelSlots)
            {
                ClearOutputSlot(slotName);
            }
            return;
        }

        const cv::Mat &inputImage = *inputData;

        try
        {
            std::vector<cv::Mat> channels;
            cv::split(inputImage, channels);

            // Set outputs based on available channels
            for (size_t i = 0; i < kChannelSlots.size(); ++i)
            {
                if (i < channels.size())
                {
                    SetOutputSlotData(kChannelSlots[i], channels[i]);
                }
                else
                {
                    ClearOutputSlot(kChannelSlots[i]);
                }
            }

            LOG_INFO("SplitChannelsNode {}: Split into {} channels", GetName(), channels.size());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("SplitChannelsNode {}: OpenCV error: {}", GetName(), e.what());
            for (const auto &slotName : kChannelSlots)
            {
                ClearOutputSlot(slotName);
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("SplitChannelsNode {}: Error processing image: {}", GetName(), e.what());
            for (const auto &slotName : kChannelSlots)
            {
                ClearOutputSlot(slotName);
            }
        }
    }
} // namespace VisionCraft::Vision::Algorithms
