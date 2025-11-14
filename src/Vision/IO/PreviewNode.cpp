#include "Vision/IO/PreviewNode.h"
#include "Logger.h"
#include "Nodes/Core/EngineConstants.h"

namespace VisionCraft::Vision::IO
{
    PreviewNode::PreviewNode(Nodes::NodeId id, const std::string &name) : Node(id, name)
    {
        CreateInputSlot("Input");
        CreateOutputSlot("Output");
    }

    void PreviewNode::Process()
    {
        auto inputData = GetInputSlot("Input").GetData<cv::Mat>();
        if (!inputData || inputData->empty())
        {
            LOG_WARN("PreviewNode {}: No input image to preview", GetName());
            inputImage = cv::Mat{};
            outputImage = cv::Mat{};
            ClearOutputSlot("Output");
            return;
        }

        inputImage = *inputData;
        outputImage = inputImage.clone();
        UpdateTexture();
        SetOutputSlotData("Output", outputImage);

        LOG_INFO("PreviewNode {}: Processing image ({}x{}, {} channels)",
            GetName(),
            outputImage.cols,
            outputImage.rows,
            outputImage.channels());
    }

    void PreviewNode::SetInputImage(const cv::Mat &image)
    {
        inputImage = image;
    }

    const cv::Mat &PreviewNode::GetOutputImage() const
    {
        return outputImage;
    }

    bool PreviewNode::HasValidImage() const
    {
        return !outputImage.empty() && texture.IsValid();
    }

    GLuint PreviewNode::GetTextureId() const
    {
        return texture.Get();
    }

    void PreviewNode::UpdateTexture()
    {
        if (outputImage.empty())
        {
            texture.Reset();
            return;
        }

        cv::Mat rgbImage;
        cv::cvtColor(outputImage, rgbImage, cv::COLOR_BGR2RGB);

        if (!texture.Create())
        {
            return;
        }

        glBindTexture(GL_TEXTURE_2D, texture.Get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, rgbImage.cols, rgbImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbImage.data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    std::pair<float, float> PreviewNode::CalculatePreviewDimensions(float nodeContentWidth,
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

    float PreviewNode::CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const
    {
        if (!HasValidImage())
        {
            return 0.0f;
        }

        auto [previewWidth, actualPreviewHeight] = CalculatePreviewDimensions(nodeContentWidth, 0);

        const float imagePreviewSpacing = Constants::ImageInputNode::Preview::kSpacing * zoomLevel;
        return actualPreviewHeight + imagePreviewSpacing;
    }

} // namespace VisionCraft::Vision::IO
