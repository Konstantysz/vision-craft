#include "ImageInputNode.h"
#include "Logger.h"

#include "../EngineConstants.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <opencv2/opencv.hpp>


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
    namespace
    {
        // Use special prefix in lastLoadedPath to store error messages
        inline constexpr std::string_view kErrorPrefix = "ERROR:";
    } // namespace

    ImageInputNode::ImageInputNode(NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("FilePath", std::filesystem::path{});
        CreateOutputSlot("Output");
        filePathBuffer[0] = '\0';
    }

    std::string ImageInputNode::GetErrorMessage() const
    {
        if (lastLoadedPath.starts_with(kErrorPrefix))
        {
            return lastLoadedPath.substr(kErrorPrefix.length());
        }
        return "";
    }

    bool ImageInputNode::HasError() const
    {
        return lastLoadedPath.starts_with(kErrorPrefix);
    }

    void ImageInputNode::Process()
    {
        auto filepath = GetInputValue<std::filesystem::path>("FilePath").value_or(std::filesystem::path{});

        if (filepath.empty())
        {
            outputImage = cv::Mat{};
            lastLoadedPath.clear();
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
        auto setError = [this](std::string_view errorMsg) {
            lastLoadedPath = std::string(kErrorPrefix) + std::string(errorMsg);
            outputImage = cv::Mat{};
        };

        try
        {
            outputImage = cv::imread(filepath, cv::IMREAD_COLOR);

            if (outputImage.empty()) [[unlikely]]
            {
                setError("Failed to load image");
                LOG_ERROR(
                    "ImageInputNode {}: Failed to load image from '{}' - file may be corrupted or unsupported format",
                    GetName(),
                    filepath);
                return;
            }

            UpdateTexture();
            lastLoadedPath = filepath;

            LOG_INFO("ImageInputNode {}: Successfully loaded image from '{}' ({}x{}, {} channels)",
                GetName(),
                filepath,
                outputImage.cols,
                outputImage.rows,
                outputImage.channels());
        }
        catch (const cv::Exception &e)
        {
            setError("OpenCV error");
            LOG_ERROR("ImageInputNode {}: OpenCV error loading image '{}': {}", GetName(), filepath, e.what());
        }
        catch (const std::exception &e)
        {
            setError("Error loading image");
            LOG_ERROR("ImageInputNode {}: Unexpected error loading image '{}': {}", GetName(), filepath, e.what());
        }
    }

    void ImageInputNode::UpdateTexture()
    {
        if (outputImage.empty())
        {
            texture.Reset();
            return;
        }

        // Safety check: ensure we have valid image data
        if (outputImage.data == nullptr || outputImage.cols <= 0 || outputImage.rows <= 0)
        {
            LOG_ERROR("ImageInputNode {}: Invalid image data, cannot create texture", GetName());
            texture.Reset();
            return;
        }

        cv::Mat rgbImage;
        try
        {
            // Verify outputImage is valid before conversion
            if (!outputImage.data)
            {
                LOG_ERROR("ImageInputNode {}: outputImage.data is null before cvtColor", GetName());
                texture.Reset();
                return;
            }

            cv::cvtColor(outputImage, rgbImage, cv::COLOR_BGR2RGB);

            // Verify conversion succeeded
            if (rgbImage.empty() || !rgbImage.data)
            {
                LOG_ERROR("ImageInputNode {}: RGB conversion produced invalid image", GetName());
                texture.Reset();
                return;
            }
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ImageInputNode {}: Failed to convert image to RGB: {}", GetName(), e.what());
            texture.Reset();
            return;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ImageInputNode {}: Standard exception during RGB conversion: {}", GetName(), e.what());
            texture.Reset();
            return;
        }

        if (!texture.Create())
        {
            LOG_ERROR("ImageInputNode {}: Failed to create OpenGL texture", GetName());
            return;
        }

        GLuint textureId = texture.Get();
        if (textureId == 0)
        {
            LOG_ERROR("ImageInputNode {}: Invalid texture ID", GetName());
            return;
        }

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, rgbImage.cols, rgbImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbImage.data);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            LOG_ERROR("ImageInputNode {}: OpenGL error after texture upload: 0x{:x}", GetName(), error);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    std::pair<float, float> ImageInputNode::CalculatePreviewDimensions(float nodeContentWidth,
        [[maybe_unused]] float maxHeight) const
    {
        if (!HasValidImage() || outputImage.rows <= 0 || outputImage.cols <= 0)
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