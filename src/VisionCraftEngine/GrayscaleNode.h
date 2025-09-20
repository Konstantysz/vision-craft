#pragma once
#include "Node.h"

#include <opencv2/core.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node that converts an image to grayscale.
     *
     * Expects input image in SetInputImage(). Result is stored in outputImage.
     */
    class GrayscaleNode : public Node
    {
    public:
        /**
         * @brief Construct a new GrayscaleNode object.
         * @param id Unique identifier for the node.
         */
        GrayscaleNode(NodeId id);

        /**
         * @brief Set input image for processing.
         */
        void SetInputImage(const cv::Mat &img);

        /**
         * @brief Process the node (convert image to grayscale).
         */
        void Process() override;

        /**
         * @brief Get the output (grayscale) image.
         */
        const cv::Mat &GetOutputImage() const;

    private:
        cv::Mat inputImage;
        cv::Mat outputImage;
    };

} // namespace VisionCraft::Engine
