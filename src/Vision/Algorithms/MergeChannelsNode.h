#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for merging image channels.
     */
    class MergeChannelsNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs MergeChannels node.
         * @param id Node ID
         * @param name Node name
         */
        MergeChannelsNode(Nodes::NodeId id, const std::string &name = "Merge Channels");

        /**
         * @brief Virtual destructor.
         */
        virtual ~MergeChannelsNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "MergeChannels";
        }

        /**
         * @brief Processes input channels and merges them.
         */
        void Process() override;

    private:
        static constexpr std::array kChannelSlots{ "Channel 1", "Channel 2", "Channel 3", "Channel 4" };
    };
} // namespace VisionCraft::Vision::Algorithms
