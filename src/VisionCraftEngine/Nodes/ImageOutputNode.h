#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for displaying and saving processed images.
     *
     * ImageOutputNode serves as the final destination for processed images in the
     * pipeline. It can display images in the results panel and optionally save
     * them to disk.
     */
    class ImageOutputNode : public Node
    {
    public:
        /**
         * @brief Constructs an ImageOutputNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        ImageOutputNode(NodeId id, const std::string &name = "Image Output");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ImageOutputNode() = default;

        /**
         * @brief Processes the input image for display/saving.
         *
         * Prepares the input image for display and optionally saves it to disk
         * if a save path is specified in the parameters.
         */
        void Process() override;

        /**
         * @brief Sets the input image for output.
         * @param image Input image to display/save
         */
        void SetInputImage(const cv::Mat &image)
        {
            inputImage = image;
        }

        /**
         * @brief Gets the image for display.
         * @return OpenCV Mat containing the image to display
         */
        [[nodiscard]] const cv::Mat &GetDisplayImage() const
        {
            return displayImage;
        }

        /**
         * @brief Checks if the node has a valid image to display.
         * @return True if display image is valid, false otherwise
         */
        [[nodiscard]] bool HasValidImage() const
        {
            return !displayImage.empty();
        }

        /**
         * @brief Gets the last save status.
         * @return True if last save operation was successful, false otherwise
         */
        [[nodiscard]] bool GetLastSaveStatus() const
        {
            return lastSaveSuccessful;
        }

    private:
        cv::Mat inputImage;              ///< Input image to process
        cv::Mat displayImage;            ///< Image prepared for display
        bool lastSaveSuccessful = false; ///< Status of last save operation

        /**
         * @brief Saves the image to the specified file path.
         * @param filepath Path where to save the image
         * @return True if save was successful, false otherwise
         */
        bool SaveImage(const std::string &filepath);
    };
} // namespace VisionCraft::Engine