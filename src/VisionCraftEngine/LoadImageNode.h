#pragma once
#include "Node.h"

#include <string>

#include <opencv2/core.hpp>

namespace VisionCraft::Engine
{

    /**
     * @brief Node that loads an image from a file.
     *
     * Parameter: filePath (path to the image file)
     * Stores loaded image in cv::Mat image.
     */
    class LoadImageNode : public Node
    {
    public:
        /**
         * @brief Construct a new LoadImageNode object.
         * @param id Unique identifier for the node.
         * @param filePath Path to the image file.
         */
        LoadImageNode(NodeId id, const std::string &filePath);

        /**
         * @brief Process the node (load image from file).
         */
        void Process() override;

        /**
         * @brief Get loaded image.
         */
        const cv::Mat &GetImage() const
        {
            return image;
        }

    private:
        cv::Mat image;
    };

} // namespace VisionCraft::Engine
