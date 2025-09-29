#include "ImageInputNode.h"
#include "Logger.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <imgui.h>
#include <opencv2/opencv.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <commdlg.h>
#include <windows.h>
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

    ImageInputNode::~ImageInputNode()
    {
        CleanupTexture();
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
            CleanupTexture();
            return;
        }

        // Clean up previous texture
        CleanupTexture();

        // Convert BGR to RGB for OpenGL
        cv::Mat rgbImage;
        cv::cvtColor(outputImage, rgbImage, cv::COLOR_BGR2RGB);

        // Generate and bind texture
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

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

    void ImageInputNode::CleanupTexture()
    {
        if (textureId != 0)
        {
            glDeleteTextures(1, &textureId);
            textureId = 0;
        }
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

    std::string ImageInputNode::RenderFilePathInput()
    {
        // Create unique ID for this node's input field
        std::string inputId = "##ImagePath_" + std::to_string(static_cast<int>(GetId()));

        if (ImGui::InputText(
                inputId.c_str(), filePathBuffer, sizeof(filePathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            // Validate that the path exists and is an image file
            std::filesystem::path filepath(filePathBuffer);
            if (std::filesystem::exists(filepath))
            {
                std::string extension = filepath.extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" || extension == ".bmp"
                    || extension == ".tiff" || extension == ".tif" || extension == ".webp")
                {
                    return std::string(filePathBuffer);
                }
            }
        }
        return "";
    }

    void ImageInputNode::RenderCustomUI()
    {
        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 55.0f;
        const float spacing = 5.0f;
        const float inputWidth = availableWidth - (buttonWidth * 2) - (spacing * 2);

        // File path input field with proper sizing
        ImGui::PushItemWidth(inputWidth);
        std::string pathFromInput = RenderFilePathInput();
        ImGui::PopItemWidth();

        // Browse button
        ImGui::SameLine(0, spacing);
        std::string browseId = "Browse##" + std::to_string(static_cast<int>(GetId()));
        if (ImGui::Button(browseId.c_str(), ImVec2(buttonWidth, 0)))
        {
            std::string selectedPath = OpenFileBrowser();
            if (!selectedPath.empty())
            {
                // Update the input buffer
                strncpy(filePathBuffer, selectedPath.c_str(), sizeof(filePathBuffer) - 1);
                filePathBuffer[sizeof(filePathBuffer) - 1] = '\0';
                pathFromInput = selectedPath;
            }
        }

        // Load button
        ImGui::SameLine(0, spacing);
        std::string loadId = "Load##" + std::to_string(static_cast<int>(GetId()));
        if (ImGui::Button(loadId.c_str(), ImVec2(buttonWidth, 0)))
        {
            if (strlen(filePathBuffer) > 0)
            {
                pathFromInput = std::string(filePathBuffer);
            }
        }

        // Process image loading if path was selected/entered
        if (!pathFromInput.empty())
        {
            SetParam("filepath", std::filesystem::path(pathFromInput));
            Process(); // Reload the image
        }

        // Display current file status (compact)
        if (!lastLoadedPath.empty())
        {
            std::string filename = std::filesystem::path(lastLoadedPath).filename().string();
            if (filename.length() > 20)
            {
                filename = filename.substr(0, 17) + "...";
            }
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "✓ %s", filename.c_str());
        }
        else if (strlen(filePathBuffer) > 0)
        {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "⚠ No image");
        }

        // Display compact image preview if available
        if (HasValidImage() && textureId != 0)
        {
            // Calculate small preview size (max 120x80 maintaining aspect ratio)
            const float maxPreviewWidth = 120.0f;
            const float maxPreviewHeight = 80.0f;

            float imageAspect = static_cast<float>(outputImage.cols) / static_cast<float>(outputImage.rows);
            float previewWidth = maxPreviewWidth;
            float previewHeight = maxPreviewWidth / imageAspect;

            if (previewHeight > maxPreviewHeight)
            {
                previewHeight = maxPreviewHeight;
                previewWidth = maxPreviewHeight * imageAspect;
            }

            // Center the image preview
            float regionWidth = ImGui::GetContentRegionAvail().x;
            if (previewWidth < regionWidth)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (regionWidth - previewWidth) * 0.5f);
            }

            ImGui::Image(
                static_cast<ImTextureID>(textureId), ImVec2(previewWidth, previewHeight), ImVec2(0, 0), ImVec2(1, 1));

            // Show dimensions on hover
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%dx%d pixels", outputImage.cols, outputImage.rows);
            }
        }
    }

    std::pair<float, float> ImageInputNode::CalculatePreviewDimensions(float nodeContentWidth, float maxHeight) const
    {
        if (!HasValidImage())
        {
            return { 0.0f, 0.0f };
        }

        // Get image aspect ratio
        float imageAspect = static_cast<float>(outputImage.cols) / static_cast<float>(outputImage.rows);

        // ALWAYS use full width and preserve aspect ratio - no distortion!
        float previewWidth = nodeContentWidth;
        float previewHeight = previewWidth / imageAspect;

        // Let height be whatever it needs to be to preserve aspect ratio
        // The node will resize itself to accommodate the image
        return { previewWidth, previewHeight };
    }

} // namespace VisionCraft::Engine