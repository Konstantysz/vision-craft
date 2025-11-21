#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::IO
{
    /**
     * @brief Node for displaying and saving processed images.
     */
    class ImageOutputNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs image output node.
         * @param id Node ID
         * @param name Node name
         */
        ImageOutputNode(Nodes::NodeId id, const std::string &name = "Image Output");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ImageOutputNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "ImageOutput";
        }

        /**
         * @brief Processes input image for display/saving.
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
         * @brief Returns display image.
         * @return Image to display
         */
        [[nodiscard]] const cv::Mat &GetDisplayImage() const
        {
            return displayImage;
        }

        /**
         * @brief Checks if node has valid image.
         * @return True if display image is valid
         */
        [[nodiscard]] bool HasValidImage() const
        {
            return !displayImage.empty();
        }

        /**
         * @brief Returns last save status.
         * @return True if last save was successful
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
         * @brief Saves image to file.
         * @param filepath Save path
         * @return True if successful
         */
        bool SaveImage(const std::string &filepath);
    };
} // namespace VisionCraft::Vision::IO
