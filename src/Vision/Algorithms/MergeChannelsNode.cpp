#include "Vision/Algorithms/MergeChannelsNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    MergeChannelsNode::MergeChannelsNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Channel 1");
        CreateInputSlot("Channel 2");
        CreateInputSlot("Channel 3");
        CreateInputSlot("Channel 4");
        CreateOutputSlot("Output");
    }

    void MergeChannelsNode::Process()
    {
        inputChannels.clear();

        auto c1 = GetInputValue<cv::Mat>("Channel 1");
        auto c2 = GetInputValue<cv::Mat>("Channel 2");
        auto c3 = GetInputValue<cv::Mat>("Channel 3");
        auto c4 = GetInputValue<cv::Mat>("Channel 4");

        // We need at least Channel 1 to determine size/type
        if (!c1 || c1->empty())
        {
            LOG_WARN("MergeChannelsNode {}: Channel 1 is required", GetName());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
            return;
        }

        inputChannels.push_back(*c1);

        // Helper to check compatibility
        auto checkCompat = [&](const cv::Mat &m, const std::string &name) -> bool {
            if (m.empty())
                return false;
            if (m.size() != inputChannels[0].size() || m.depth() != inputChannels[0].depth())
            {
                LOG_WARN("MergeChannelsNode {}: {} size/depth mismatch, ignoring", GetName(), name);
                return false;
            }
            return true;
        };

        if (c2 && checkCompat(*c2, "Channel 2"))
            inputChannels.push_back(*c2);
        if (c3 && checkCompat(*c3, "Channel 3"))
            inputChannels.push_back(*c3);
        if (c4 && checkCompat(*c4, "Channel 4"))
            inputChannels.push_back(*c4);

        try
        {
            cv::merge(inputChannels, outputImage);
            SetOutputSlotData("Output", outputImage);

            LOG_INFO("MergeChannelsNode {}: Merged {} channels", GetName(), inputChannels.size());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("MergeChannelsNode {}: OpenCV error: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("MergeChannelsNode {}: Error processing image: {}", GetName(), e.what());
            outputImage = cv::Mat();
            ClearOutputSlot("Output");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
