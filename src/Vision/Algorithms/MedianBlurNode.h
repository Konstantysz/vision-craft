#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for Median Blur.
     */
    class MedianBlurNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs Median Blur node.
         * @param id Node ID
         * @param name Node name
         */
        MedianBlurNode(Nodes::NodeId id, const std::string &name = "Median Blur");

        /**
         * @brief Virtual destructor.
         */
        virtual ~MedianBlurNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "MedianBlurNode";
        }

        /**
         * @brief Processes input image using Median Blur.
         */
        void Process() override;

    private:
        cv::Mat inputImage;  ///< Input image
        cv::Mat outputImage; ///< Resulting image
    };
} // namespace VisionCraft::Vision::Algorithms
