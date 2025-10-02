#pragma once

#include "Node.h"
#include <glad/glad.h>
#include <opencv2/opencv.hpp>
#include <string>

#include "EngineConstants.h"
#include "Texture.h"

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
        ImageInputNode(NodeId id, const std::string &name = "Image Input");


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
        [[nodiscard]] const cv::Mat &GetOutputImage() const
        {
            return outputImage;
        }

        /**
         * @brief Checks if an image has been successfully loaded AND texture is ready.
         * @return True if image is loaded and texture is available for rendering, false otherwise
         */
        [[nodiscard]] bool HasValidImage() const;

        /**
         * @brief Gets the OpenGL texture ID for displaying the image.
         * @return OpenGL texture ID, or 0 if no texture is available
         */
        [[nodiscard]] GLuint GetTextureId() const;


        /**
         * @brief Calculates the actual preview dimensions that will be used for rendering.
         * @param nodeContentWidth Available width for the preview (node width minus padding)
         * @param maxHeight Maximum allowed height for the preview
         * @return Pair of (width, height) for the actual preview dimensions
         */
        [[nodiscard]] std::pair<float, float> CalculatePreviewDimensions(float nodeContentWidth, float maxHeight) const;


        /**
         * @brief Calculates extra height needed for image preview.
         * Overrides base class to provide polymorphic dimension calculation.
         * @param nodeContentWidth Available content width
         * @param zoomLevel Current zoom level
         * @return Extra height required for image preview
         */
        [[nodiscard]] float CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const override;

    private:
        /**
         * @brief Loads image from the specified file path.
         * @param filepath Path to the image file
         */
        void LoadImageFromPath(const std::string &filepath);

        /**
         * @brief Creates/updates OpenGL texture from the loaded image.
         */
        void UpdateTexture();


        cv::Mat outputImage;        ///< Loaded image data
        Core::Texture texture;      ///< RAII-managed OpenGL texture for display
        std::string lastLoadedPath; ///< Last successfully loaded file path
        char filePathBuffer[Constants::Buffers::kFilePathBufferSize] =
            ""; ///< Buffer for file path input (ImGui requirement)
    };
} // namespace VisionCraft::Engine