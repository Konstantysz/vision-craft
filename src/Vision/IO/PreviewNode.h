#pragma once

#include "Nodes/Core/Node.h"
#include "Texture.h"
#include <glad/glad.h>
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::IO
{
    /**
     * @brief Node for previewing images while passing them through.
     */
    class PreviewNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs preview node.
         * @param id Node ID
         * @param name Node name
         */
        PreviewNode(Nodes::NodeId id, const std::string &name = "Preview");

        /**
         * @brief Virtual destructor.
         */
        virtual ~PreviewNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "Preview";
        }

        /**
         * @brief Processes node by passing input to output.
         */
        void Process() override;

        /**
         * @brief Sets input image.
         * @param image Input image
         */
        void SetInputImage(const cv::Mat &image);

        /**
         * @brief Returns output image.
         * @return Pass-through image
         */
        [[nodiscard]] const cv::Mat &GetOutputImage() const;

        /**
         * @brief Checks if image is available for preview.
         * @return True if valid and texture is ready
         */
        [[nodiscard]] bool HasValidImage() const;

        /**
         * @brief Returns OpenGL texture ID.
         * @return Texture ID
         */
        [[nodiscard]] GLuint GetTextureId() const;

        /**
         * @brief Calculates preview dimensions for rendering.
         * @param nodeContentWidth Available content width
         * @param maxHeight Maximum height
         * @return Preview dimensions (width, height)
         */
        [[nodiscard]] std::pair<float, float> CalculatePreviewDimensions(float nodeContentWidth, float maxHeight) const;

        /**
         * @brief Calculates extra height for image preview.
         * @param nodeContentWidth Available content width
         * @param zoomLevel Current zoom level
         * @return Extra height required
         */
        [[nodiscard]] float CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const override;

        /**
         * @brief Updates OpenGL texture from current image.
         * @note Must be called on the main thread (OpenGL context thread).
         */
        void UpdateTexture();

    private:
        cv::Mat inputImage;     ///< Input image from connected node
        cv::Mat outputImage;    ///< Output image (same as input)
        Kappa::Texture texture; ///< RAII-managed OpenGL texture for display
    };
} // namespace VisionCraft::Vision::IO
