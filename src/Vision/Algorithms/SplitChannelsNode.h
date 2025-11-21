#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for splitting image channels.
     */
    class SplitChannelsNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs SplitChannels node.
         * @param id Node ID
         * @param name Node name
         */
        SplitChannelsNode(Nodes::NodeId id, const std::string &name = "Split Channels");

        /**
         * @brief Virtual destructor.
         */
        virtual ~SplitChannelsNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "SplitChannels";
        }

        /**
         * @brief Processes input image and splits channels.
         */
        void Process() override;

    private:
        static constexpr std::array kChannelSlots{ "Channel 1", "Channel 2", "Channel 3", "Channel 4" };
    };
} // namespace VisionCraft::Vision::Algorithms
