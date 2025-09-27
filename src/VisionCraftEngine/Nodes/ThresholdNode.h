#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for image thresholding operations.
     *
     * ThresholdNode applies various thresholding techniques to input images,
     * converting them to binary images based on configurable threshold values
     * and methods.
     */
    class ThresholdNode : public Node
    {
    public:
        /**
         * @brief Constructs a ThresholdNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        ThresholdNode(NodeId id, const std::string &name = "Threshold");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ThresholdNode() = default;

        /**
         * @brief Processes the input image using thresholding.
         *
         * Applies thresholding with configurable threshold value, maximum value,
         * and thresholding type. The input image is automatically converted to
         * grayscale if needed.
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
         * @brief Gets the processed threshold image.
         * @return OpenCV Mat containing the thresholded binary image
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
        cv::Mat inputImage;  ///< Input image for thresholding
        cv::Mat outputImage; ///< Resulting thresholded image

        /**
         * @brief Converts threshold type string to OpenCV constant.
         * @param typeStr String representation of threshold type
         * @return OpenCV threshold type constant
         */
        int GetThresholdType(const std::string &typeStr) const;
    };
} // namespace VisionCraft::Engine