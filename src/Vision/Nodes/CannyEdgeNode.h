#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for Canny edge detection.
     */
    class CannyEdgeNode : public Node
    {
    public:
        /**
         * @brief Constructs Canny edge detection node.
         * @param id Node ID
         * @param name Node name
         */
        CannyEdgeNode(NodeId id, const std::string &name = "Canny Edge Detection");

        /**
         * @brief Virtual destructor.
         */
        virtual ~CannyEdgeNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "CannyEdgeNode";
        }

        /**
         * @brief Processes input image using Canny edge detection.
         */
        void Process() override;

        /**
         * @brief Sets input image.
         * @param image Input image
         */
        void SetInputImage(const cv::Mat &image)
        {
            inputImage = image;
        }

        /**
         * @brief Returns processed edge image.
         * @return Edge-detected image
         */
        const cv::Mat &GetOutputImage() const
        {
            return outputImage;
        }

        /**
         * @brief Checks if node has valid output.
         * @return True if output is valid
         */
        bool HasValidOutput() const
        {
            return !outputImage.empty();
        }

    private:
        cv::Mat inputImage;  ///< Input image for edge detection
        cv::Mat outputImage; ///< Resulting edge-detected image
    };
} // namespace VisionCraft::Engine
