#include "ImageInputNode.h"
#include "Logger.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <opencv2/opencv.hpp>

// Include Engine constants
#include "../EngineConstants.h"


#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off
#include <windows.h>
#include <commdlg.h>
// clang-format on
#else
#include <cstdlib>
#include <cstring>
#endif

namespace VisionCraft::Engine
{
    ImageInputNode::ImageInputNode(NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("FilePath", std::filesystem::path{});
        CreateOutputSlot("Output");
        filePathBuffer[0] = '\0';
    }

    void ImageInputNode::Process()
    {
        auto filepath = GetInputValue<std::filesystem::path>("FilePath").value_or(std::filesystem::path{});

        if (filepath.empty())
        {
            outputImage = cv::Mat{};
            ClearOutputSlot("Output");
            return;
        }

        LoadImageFromPath(filepath.string());

        if (!outputImage.empty())
        {
            SetOutputSlotData("Output", outputImage);
        }
        else
        {
            ClearOutputSlot("Output");
        }
    }

    bool ImageInputNode::HasValidImage() const
    {
        return !outputImage.empty() && texture.IsValid();
    }

    GLuint ImageInputNode::GetTextureId() const
    {
        return texture.Get();
    }

    void ImageInputNode::LoadImageFromPath(const std::string &filepath)
    {
        try
        {
            outputImage = cv::imread(filepath, cv::IMREAD_COLOR);

            if (outputImage.empty()) [[unlikely]]
            {
                LOG_ERROR(
                    "ImageInputNode {}: Failed to load image from '{}' - file may be corrupted or unsupported format",
                    GetName(),
                    filepath);
                return;
            }

            // Update texture for display
            UpdateTexture();
            lastLoadedPath = filepath;

            // Success logging with structured information
            LOG_INFO("ImageInputNode {}: Successfully loaded image from '{}' ({}x{}, {} channels, {} bytes)",
                GetName(),
                filepath,
                outputImage.cols,
                outputImage.rows,
                outputImage.channels(),
                outputImage.total() * outputImage.elemSize());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ImageInputNode {}: OpenCV error loading image '{}': {}", GetName(), filepath, e.what());
            outputImage = cv::Mat{};
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ImageInputNode {}: Unexpected error loading image '{}': {}", GetName(), filepath, e.what());
            outputImage = cv::Mat{};
        }
    }

    void ImageInputNode::UpdateTexture()
    {
        if (outputImage.empty())
        {
            texture.Reset(); // RAII automatically handles cleanup
            return;
        }

        // Convert BGR to RGB for OpenGL
        cv::Mat rgbImage;
        cv::cvtColor(outputImage, rgbImage, cv::COLOR_BGR2RGB);

        // Create new texture (automatically cleans up previous)
        if (!texture.Create())
        {
            return;
        }

        // Bind and configure texture
        glBindTexture(GL_TEXTURE_2D, texture.Get());

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Upload texture data
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, rgbImage.cols, rgbImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbImage.data);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    std::pair<float, float> ImageInputNode::CalculatePreviewDimensions(float nodeContentWidth,
        [[maybe_unused]] float maxHeight) const
    {
        if (!HasValidImage())
        {
            return { 0.0f, 0.0f };
        }

        float imageAspect = static_cast<float>(outputImage.cols) / static_cast<float>(outputImage.rows);

        float previewWidth = nodeContentWidth;
        float previewHeight = previewWidth / imageAspect;

        return { previewWidth, previewHeight };
    }


    float ImageInputNode::CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const
    {
        if (!HasValidImage())
        {
            return 0.0f;
        }

        auto [previewWidth, actualPreviewHeight] = CalculatePreviewDimensions(nodeContentWidth, 0);

        const float imagePreviewSpacing = Constants::ImageInputNode::Preview::kSpacing * zoomLevel;
        return actualPreviewHeight + imagePreviewSpacing;
    }

} // namespace VisionCraft::Engine