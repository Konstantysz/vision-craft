#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for image thresholding operations.
     */
    class ThresholdNode : public Node
    {
    public:
        /**
         * @brief Constructs threshold node.
         * @param id Node ID
         * @param name Node name
         */
        ThresholdNode(NodeId id, const std::string &name = "Threshold");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ThresholdNode() = default;

        /**
         * @brief Processes input image using thresholding.
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
         * @brief Returns processed image.
         * @return Thresholded image
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
        cv::Mat inputImage;  ///< Input image
        cv::Mat outputImage; ///< Thresholded image

        /**
         * @brief Converts threshold type string to OpenCV constant.
         * @param typeStr Threshold type string
         * @return OpenCV threshold type constant
         */
        int GetThresholdType(const std::string &typeStr) const;
    };
} // namespace VisionCraft::Engine
