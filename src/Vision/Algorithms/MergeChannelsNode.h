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
            return "MergeChannelsNode";
        }

        /**
         * @brief Processes input channels and merges them.
         */
        void Process() override;

    private:
        std::vector<cv::Mat> inputChannels; ///< Input channels
        cv::Mat outputImage;                ///< Resulting image
    };
} // namespace VisionCraft::Vision::Algorithms
