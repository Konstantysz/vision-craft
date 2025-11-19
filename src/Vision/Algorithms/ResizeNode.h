#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for Image Resizing.
     */
    class ResizeNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs Resize node.
         * @param id Node ID
         * @param name Node name
         */
        ResizeNode(Nodes::NodeId id, const std::string &name = "Resize");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ResizeNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "ResizeNode";
        }

        /**
         * @brief Processes input image using Resize.
         */
        void Process() override;

    private:
        cv::Mat inputImage;  ///< Input image
        cv::Mat outputImage; ///< Resulting image
    };
} // namespace VisionCraft::Vision::Algorithms
