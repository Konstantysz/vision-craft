#include "ImageOutputNode.h"
#include "Logger.h"
#include <filesystem>

namespace VisionCraft::Engine
{
    ImageOutputNode::ImageOutputNode(NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateInputSlot("SavePath", std::filesystem::path{});
        CreateInputSlot("AutoSave", false);
        CreateInputSlot("Format", std::string{ "png" });
    }

    void ImageOutputNode::Process()
    {
        auto inputData = GetInputValue<cv::Mat>("Input");
        if (!inputData || inputData->empty())
        {
            LOG_WARN("ImageOutputNode {}: No input image provided", GetName());
            inputImage = cv::Mat();
            displayImage = cv::Mat();
            return;
        }

        inputImage = *inputData;

        try
        {
            displayImage = inputImage.clone();

            const auto autoSave = GetInputValue<bool>("AutoSave").value_or(false);
            const auto savePath = GetInputValue<std::filesystem::path>("SavePath").value_or(std::filesystem::path{});

            if (autoSave && !savePath.empty())
            {
                lastSaveSuccessful = SaveImage(savePath.string());
            }
            else
            {
                lastSaveSuccessful = false;
            }

            LOG_INFO("ImageOutputNode {}: Processed image ({}x{}, {} channels)",
                GetName(),
                displayImage.cols,
                displayImage.rows,
                displayImage.channels());
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ImageOutputNode {}: OpenCV error: {}", GetName(), e.what());
            displayImage = cv::Mat();
            lastSaveSuccessful = false;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ImageOutputNode {}: Error processing image: {}", GetName(), e.what());
            displayImage = cv::Mat();
            lastSaveSuccessful = false;
        }
    }

    bool ImageOutputNode::SaveImage(const std::string &filepath)
    {
        if (displayImage.empty())
        {
            LOG_ERROR("ImageOutputNode {}: Cannot save empty image", GetName());
            return false;
        }

        try
        {
            std::filesystem::path path(filepath);
            std::filesystem::path dir = path.parent_path();

            if (!dir.empty() && !std::filesystem::exists(dir))
            {
                std::filesystem::create_directories(dir);
                LOG_INFO("ImageOutputNode {}: Created directory '{}'", GetName(), dir.string());
            }

            const auto format = GetInputValue<std::string>("Format").value_or("png");

            std::vector<int> compressionParams;
            if (format == "jpg" || format == "jpeg")
            {
                compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
                compressionParams.push_back(95);
            }
            else if (format == "png")
            {
                compressionParams.push_back(cv::IMWRITE_PNG_COMPRESSION);
                compressionParams.push_back(3);
            }

            bool success = cv::imwrite(filepath, displayImage, compressionParams);

            if (success)
            {
                LOG_INFO("ImageOutputNode {}: Successfully saved image to '{}'", GetName(), filepath);
            }
            else
            {
                LOG_ERROR("ImageOutputNode {}: Failed to save image to '{}'", GetName(), filepath);
            }

            return success;
        }
        catch (const cv::Exception &e)
        {
            LOG_ERROR("ImageOutputNode {}: OpenCV error saving to '{}': {}", GetName(), filepath, e.what());
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("ImageOutputNode {}: Error saving to '{}': {}", GetName(), filepath, e.what());
            return false;
        }
    }
} // namespace VisionCraft::Engine
