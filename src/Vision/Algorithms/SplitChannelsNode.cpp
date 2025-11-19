#include "Vision/Algorithms/SplitChannelsNode.h"
#include "Logger.h"
#include <stdexcept>

namespace VisionCraft::Vision::Algorithms
{
    SplitChannelsNode::SplitChannelsNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateOutputSlot("Channel 1");
        CreateOutputSlot("Channel 2");
        CreateOutputSlot("Channel 3");
        CreateOutputSlot("Channel 4");
    }

    void SplitChannelsNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("SplitChannelsNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            outputChannels.clear();
            ClearOutputSlot("Channel 1");
            ClearOutputSlot("Channel 2");
            ClearOutputSlot("Channel 3");
            ClearOutputSlot("Channel 4");
            return;
        }

        inputImage = *inputData;

        try
        {
            outputChannels.clear();
            cv::split(inputImage, outputChannels);

            // Set outputs based on available channels
            if (outputChannels.size() >= 1)
                SetOutputSlotData("Channel 1", outputChannels[0]);
            else
                ClearOutputSlot("Channel 1");

            if (outputChannels.size() >= 2)
                SetOutputSlotData("Channel 2", outputChannels[1]);
            else
                ClearOutputSlot("Channel 2");

            if (outputChannels.size() >= 3)
                SetOutputSlotData("Channel 3", outputChannels[2]);
            else
                ClearOutputSlot("Channel 3");

            if (outputChannels.size() >= 4)
                SetOutputSlotData("Channel 4", outputChannels[3]);
            else
                ClearOutputSlot("Channel 4");

            LOG_INFO("SplitChannelsNode {}: Split into {} channels", GetName(), outputChannels.size());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("SplitChannelsNode {}: OpenCV error: {}", GetName(), e.what());
            outputChannels.clear();
            ClearOutputSlot("Channel 1");
            ClearOutputSlot("Channel 2");
            ClearOutputSlot("Channel 3");
            ClearOutputSlot("Channel 4");
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("SplitChannelsNode {}: Error processing image: {}", GetName(), e.what());
            outputChannels.clear();
            ClearOutputSlot("Channel 1");
            ClearOutputSlot("Channel 2");
            ClearOutputSlot("Channel 3");
            ClearOutputSlot("Channel 4");
        }
    }
} // namespace VisionCraft::Vision::Algorithms
