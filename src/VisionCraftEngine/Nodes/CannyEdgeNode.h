#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for Canny edge detection.
     *
     * CannyEdgeNode applies the Canny edge detection algorithm to input images.
     * It provides configurable threshold parameters for fine-tuning edge detection
     * sensitivity and quality.
     */
    class CannyEdgeNode : public Node
    {
    public:
        /**
         * @brief Constructs a CannyEdgeNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        CannyEdgeNode(NodeId id, const std::string &name = "Canny Edge Detection");

        /**
         * @brief Virtual destructor.
         */
        virtual ~CannyEdgeNode() = default;

        /**
         * @brief Processes the input image using Canny edge detection.
         *
         * Applies Canny edge detection with configurable low and high thresholds.
         * The input image is automatically converted to grayscale if needed.
         */
        void Process() override;

        /**
         * @brief Sets the input image for processing.
         * @param image Input image to process
         */
        void SetInputImage(const cv::Mat &image)
        {
            inputImage = image;
        }

        /**
         * @brief Gets the processed edge image.
         * @return OpenCV Mat containing the edge-detected image
         */
        const cv::Mat &GetOutputImage() const
        {
            return outputImage;
        }

        /**
         * @brief Checks if the node has valid output.
         * @return True if output image is valid, false otherwise
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