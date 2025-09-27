#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for converting images to grayscale.
     *
     * GrayscaleNode converts color images to grayscale using OpenCV's color
     * conversion functions. It automatically handles different input formats
     * and provides options for different conversion methods.
     */
    class GrayscaleNode : public Node
    {
    public:
        /**
         * @brief Constructs a GrayscaleNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        GrayscaleNode(NodeId id, const std::string &name = "Grayscale");

        /**
         * @brief Virtual destructor.
         */
        virtual ~GrayscaleNode() = default;

        /**
         * @brief Processes the input image by converting it to grayscale.
         *
         * Converts the input image to grayscale using the specified conversion method.
         * If the input is already grayscale, it passes through unchanged.
         */
        void Process() override;

        /**
         * @brief Sets the input image for processing.
         * @param image Input image to convert to grayscale
         */
        void SetInputImage(const cv::Mat &image)
        {
            inputImage = image;
        }

        /**
         * @brief Gets the processed grayscale image.
         * @return OpenCV Mat containing the grayscale image
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
        cv::Mat inputImage;  ///< Input image for grayscale conversion
        cv::Mat outputImage; ///< Resulting grayscale image

        /**
         * @brief Converts conversion method string to OpenCV constant.
         * @param methodStr String representation of conversion method
         * @return OpenCV color conversion constant
         */
        int GetConversionMethod(const std::string &methodStr) const;
    };
} // namespace VisionCraft::Engine