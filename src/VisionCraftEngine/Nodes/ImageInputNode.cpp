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
        // Set default parameters using modern type-safe API
        SetParam("filepath", std::filesystem::path{});

        // Initialize file path buffer
        filePathBuffer[0] = '\0';
    }

    void ImageInputNode::Process()
    {
        // Modern file path validation with specific configuration
        FilePathValidation kImageFileValidation;
        kImageFileValidation.mustExist = true;
        kImageFileValidation.allowEmpty = false;
        kImageFileValidation.allowedExtensions = { ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp" };

        if (!ValidateFilePath("filepath", kImageFileValidation))
        {
            outputImage = cv::Mat{};
            return;
        }

        // Type-safe parameter retrieval with filesystem::path
        const auto filepath = GetPath("filepath");

        if (filepath.empty()) [[unlikely]]
        {
            LOG_ERROR("ImageInputNode {}: Filepath parameter is empty", GetName());
            outputImage = cv::Mat{};
            return;
        }

        LoadImageFromPath(filepath.string());
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


    std::string ImageInputNode::OpenFileBrowser()
    {
        // Simple approach: use system command for file dialog
        // This avoids Windows header conflicts while still providing a file dialog
#ifdef _WIN32
        // Use PowerShell to open file dialog
        std::string command =
            R"(powershell -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.OpenFileDialog; $f.Filter = 'Image Files|*.jpg;*.jpeg;*.png;*.bmp;*.tiff;*.tif;*.webp|All Files|*.*'; $f.Title = 'Select Image File'; if($f.ShowDialog() -eq 'OK') { $f.FileName }")";

        FILE *pipe = _popen(command.c_str(), "r");
        if (!pipe)
            return "";

        char buffer[512];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        _pclose(pipe);

        // Remove trailing whitespace/newlines
        result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);

        return result.empty() ? "" : result;
#else
        // Linux: use zenity if available
        std::string command =
            "zenity --file-selection --file-filter='Image files | *.jpg *.jpeg *.png *.bmp *.tiff *.tif *.webp' "
            "2>/dev/null";
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe)
            return "";

        char buffer[512];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);

        // Remove trailing newlines
        if (!result.empty() && result.back() == '\n')
        {
            result.pop_back();
        }

        return result.empty() ? "" : result;
#endif
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