#include "ImageInputNode.h"
#include "Logger.h"

#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    ImageInputNode::ImageInputNode(NodeId id, const std::string& name)
        : Node(id, name)
    {
        // Set default parameters using modern type-safe API
        SetParam("filepath", std::filesystem::path{});
    }

    void ImageInputNode::Process()
    {
        // Modern file path validation with specific configuration
        FilePathValidation kImageFileValidation;
        kImageFileValidation.mustExist = true;
        kImageFileValidation.allowEmpty = false;
        kImageFileValidation.allowedExtensions = {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp"};

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

    void ImageInputNode::LoadImageFromPath(const std::string& filepath)
    {
        try
        {
            outputImage = cv::imread(filepath, cv::IMREAD_COLOR);

            if (outputImage.empty()) [[unlikely]]
            {
                LOG_ERROR("ImageInputNode {}: Failed to load image from '{}' - file may be corrupted or unsupported format",
                         GetName(), filepath);
                return;
            }

            // Success logging with structured information
            LOG_INFO("ImageInputNode {}: Successfully loaded image from '{}' ({}x{}, {} channels, {} bytes)",
                     GetName(),
                     filepath,
                     outputImage.cols,
                     outputImage.rows,
                     outputImage.channels(),
                     outputImage.total() * outputImage.elemSize());
        }
        catch (const cv::Exception& e)
        {
            LOG_ERROR("ImageInputNode {}: OpenCV error loading image '{}': {}",
                      GetName(), filepath, e.what());
            outputImage = cv::Mat{};
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ImageInputNode {}: Unexpected error loading image '{}': {}",
                      GetName(), filepath, e.what());
            outputImage = cv::Mat{};
        }
    }

} // namespace VisionCraft::Engine