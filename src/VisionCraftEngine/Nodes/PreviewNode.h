#pragma once

#include "Node.h"
#include "Texture.h"
#include <glad/glad.h>
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for previewing images while passing them through.
     *
     * PreviewNode acts as a passthrough node that displays the input image
     * in the GUI while forwarding it unchanged to its output. This allows
     * users to visualize intermediate results in the processing pipeline
     * without affecting the data flow.
     */
    class PreviewNode : public Node
    {
    public:
        /**
         * @brief Constructs a PreviewNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        PreviewNode(NodeId id, const std::string &name = "Preview");

        /**
         * @brief Virtual destructor.
         */
        virtual ~PreviewNode() = default;

        /**
         * @brief Processes the node by passing input to output.
         *
         * Simply forwards the input image to the output without modification,
         * while updating the internal texture for GUI display.
         */
        void Process() override;

        /**
         * @brief Sets the input image for preview.
         * @param image Input image to preview and pass through
         */
        void SetInputImage(const cv::Mat &image);

        /**
         * @brief Gets the output image (same as input).
         * @return OpenCV Mat containing the pass-through image
         */
        [[nodiscard]] const cv::Mat &GetOutputImage() const;

        /**
         * @brief Checks if an image is available for preview.
         * @return True if image is valid and texture is ready for rendering
         */
        [[nodiscard]] bool HasValidImage() const;

        /**
         * @brief Gets the OpenGL texture ID for displaying the preview.
         * @return OpenGL texture ID, or 0 if no texture is available
         */
        [[nodiscard]] GLuint GetTextureId() const;

        /**
         * @brief Calculates the actual preview dimensions for rendering.
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
         * @brief Updates the OpenGL texture from the current image.
         */
        void UpdateTexture();

        cv::Mat inputImage;    ///< Input image from connected node
        cv::Mat outputImage;   ///< Output image (same as input)
        Core::Texture texture; ///< RAII-managed OpenGL texture for display
    };
} // namespace VisionCraft::Engine
