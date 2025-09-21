#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for loading images from file system (legacy version).
     *
     * LoadImageNode allows users to load images from disk and provides them
     * as output to other nodes in the processing pipeline. It supports common
     * image formats through OpenCV.
     *
     * @note This is a legacy node. Consider using ImageInputNode for new implementations.
     */
    class LoadImageNode : public Node
    {
    public:
        /**
         * @brief Constructs a LoadImageNode with the given ID and file path.
         * @param id Unique identifier for this node
         * @param filePath Path to the image file to load
         */
        LoadImageNode(NodeId id, const std::string& filePath);

        /**
         * @brief Virtual destructor.
         */
        virtual ~LoadImageNode() = default;

        /**
         * @brief Processes the node by loading the specified image.
         *
         * Loads an image from the file path specified in the "filePath" parameter.
         * The loaded image is stored internally and made available to connected nodes.
         */
        void Process() override;

        /**
         * @brief Gets the loaded image data.
         * @return OpenCV Mat containing the loaded image, or empty Mat if no image loaded
         */
        const cv::Mat& GetImage() const { return image; }

        /**
         * @brief Checks if an image has been successfully loaded.
         * @return True if image is loaded and valid, false otherwise
         */
        bool HasValidImage() const { return !image.empty(); }

    private:
        cv::Mat image; ///< Loaded image data
    };
} // namespace VisionCraft::Engine