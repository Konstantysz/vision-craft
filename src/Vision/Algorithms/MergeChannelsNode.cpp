#include "Vision/Algorithms/MergeChannelsNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    MergeChannelsNode::MergeChannelsNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        // Execution pins
        CreateExecutionInputPin("Execute");
        CreateExecutionOutputPin("Then");

        // Data pins
        for (const auto &slotName : kChannelSlots)
        {
            CreateInputSlot(slotName);
        }
        CreateOutputSlot("Output");
    }

    void MergeChannelsNode::Process()
    {
        std::vector<cv::Mat> channels;

        // Get all channel inputs
        const std::array channelInputs{ GetInputValue<cv::Mat>(kChannelSlots[0]),
            GetInputValue<cv::Mat>(kChannelSlots[1]),
            GetInputValue<cv::Mat>(kChannelSlots[2]),
            GetInputValue<cv::Mat>(kChannelSlots[3]) };

        // Channel 1 is required
        if (!channelInputs[0] || channelInputs[0]->empty()) [[unlikely]]
        {
            LOG_WARN("MergeChannelsNode {}: Channel 1 is required", GetName());
            ClearOutputSlot("Output");
            return;
        }

        channels.push_back(*channelInputs[0]);

        // Check compatibility and add other channels
        auto checkCompat = [&](const cv::Mat &m) {
            return !m.empty() && m.size() == channels[0].size() && m.depth() == channels[0].depth();
        };

        for (size_t i = 1; i < channelInputs.size(); ++i)
        {
            if (channelInputs[i] && checkCompat(*channelInputs[i]))
            {
                channels.push_back(*channelInputs[i]);
            }
            else if (channelInputs[i] && !channelInputs[i]->empty()) [[unlikely]]
            {
                LOG_WARN("MergeChannelsNode {}: {} size/depth mismatch, ignoring", GetName(), kChannelSlots[i]);
            }
        }

        try
        {
            cv::Mat outputImage;
            cv::merge(channels, outputImage);
            SetOutputSlotData("Output", std::move(outputImage));

            LOG_INFO("MergeChannelsNode {}: Merged {} channels", GetName(), channels.size());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MergeChannelsNode {}: OpenCV error: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MergeChannelsNode {}: Error processing image: {}", GetName(), e.what());
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
