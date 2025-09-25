#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for loading images from file system.
     *
     * ImageInputNode allows users to load images from disk and provides them
     * as output to other nodes in the processing pipeline. It supports common
     * image formats through OpenCV.
     */
    class ImageInputNode : public Node
    {
    public:
        /**
         * @brief Constructs an ImageInputNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        ImageInputNode(NodeId id, const std::string& name = "Image Input");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ImageInputNode() = default;

        /**
         * @brief Processes the node by loading the specified image.
         *
         * Loads an image from the file path specified in the "filepath" parameter.
         * The loaded image is stored internally and made available to connected nodes.
         */
        void Process() override;

        /**
         * @brief Gets the loaded image data.
         * @return OpenCV Mat containing the loaded image, or empty Mat if no image loaded
         */
        const cv::Mat& GetOutputImage() const { return outputImage; }

        /**
         * @brief Checks if an image has been successfully loaded.
         * @return True if image is loaded and valid, false otherwise
         */
        bool HasValidImage() const { return !outputImage.empty(); }

    private:
        /**
         * @brief Loads image from the specified file path.
         * @param filepath Path to the image file
         */
        void LoadImageFromPath(const std::string& filepath);

        cv::Mat outputImage; ///< Loaded image data
    };
} // namespace VisionCraft::Engine