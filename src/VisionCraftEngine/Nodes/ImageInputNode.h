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
     */
    class ImageInputNode : public Node
    {
    public:
        /**
         * @brief Constructs image input node.
         * @param id Node ID
         * @param name Node name
         */
        ImageInputNode(NodeId id, const std::string &name = "Image Input");

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "ImageInputNode";
        }

        /**
         * @brief Processes node by loading specified image.
         */
        void Process() override;

        /**
         * @brief Returns loaded image.
         * @return Loaded image
         */
        [[nodiscard]] const cv::Mat &GetOutputImage() const
        {
            return outputImage;
        }

        /**
         * @brief Checks if image is loaded and texture is ready.
         * @return True if valid
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
         * @brief Returns error message if loading failed.
         * @return Error message or empty string
         */
        [[nodiscard]] std::string GetErrorMessage() const;

        /**
         * @brief Checks if node has an error.
         * @return True if error exists
         */
        [[nodiscard]] bool HasError() const;

    private:
        /**
         * @brief Loads image from file path.
         * @param filepath Image file path
         */
        void LoadImageFromPath(const std::string &filepath);

        /**
         * @brief Updates OpenGL texture from loaded image.
         */
        void UpdateTexture();


        cv::Mat outputImage;        ///< Loaded image data
        Kappa::Texture texture;     ///< RAII-managed OpenGL texture for display
        std::string lastLoadedPath; ///< Last successfully loaded file path
        char filePathBuffer[Constants::Buffers::kFilePathBufferSize] =
            ""; ///< Buffer for file path input (ImGui requirement)
    };
} // namespace VisionCraft::Engine