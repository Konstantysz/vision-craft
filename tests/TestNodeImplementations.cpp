#include "VisionCraftEngine/Nodes/CannyEdgeNode.h"
#include "VisionCraftEngine/Nodes/GrayscaleNode.h"
#include "VisionCraftEngine/Nodes/ThresholdNode.h"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace VisionCraft::Engine;

class NodeImplementationTest : public ::testing::Test
{
protected:
    cv::Mat CreateTestImage()
    {
        cv::Mat img(100, 100, CV_8UC3, cv::Scalar(128, 128, 128));
        cv::rectangle(img, cv::Point(25, 25), cv::Point(75, 75), cv::Scalar(255, 255, 255), -1);
        return img;
    }

    cv::Mat CreateGrayscaleTestImage()
    {
        cv::Mat img(100, 100, CV_8UC1, cv::Scalar(128));
        cv::rectangle(img, cv::Point(25, 25), cv::Point(75, 75), cv::Scalar(255), -1);
        return img;
    }
};

// ============================================================================
// ThresholdNode Tests
// ============================================================================

TEST_F(NodeImplementationTest, ThresholdNodeBasicProcessing)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("Threshold", 127.0);
    node.SetInputSlotData("MaxValue", 255.0);
    node.SetInputSlotData("Type", std::string("THRESH_BINARY"));

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
    EXPECT_EQ(output->rows, inputImage.rows);
    EXPECT_EQ(output->cols, inputImage.cols);
    EXPECT_EQ(output->type(), CV_8UC1);

    // Verify binary thresholding: values should be either 0 or 255
    for (int r = 0; r < output->rows; r++)
    {
        for (int c = 0; c < output->cols; c++)
        {
            uchar val = output->at<uchar>(r, c);
            EXPECT_TRUE(val == 0 || val == 255);
        }
    }
}

TEST_F(NodeImplementationTest, ThresholdNodeWithDefaults)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    // Use default threshold (127.0), maxValue (255.0), and type (THRESH_BINARY)

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

TEST_F(NodeImplementationTest, ThresholdNodeColorImageConversion)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat colorImage = CreateTestImage();

    node.SetInputSlotData("Input", colorImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    // Should convert to grayscale internally before thresholding
    EXPECT_EQ(output->type(), CV_8UC1);
}

TEST_F(NodeImplementationTest, ThresholdNodeNoInput)
{
    ThresholdNode node(1, "Threshold");

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    // Should clear output when no input
    EXPECT_FALSE(output.has_value() && !output->empty());
}

TEST_F(NodeImplementationTest, ThresholdNodeEmptyImage)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat emptyImage;

    node.SetInputSlotData("Input", emptyImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    EXPECT_FALSE(output.has_value() && !output->empty());
}

TEST_F(NodeImplementationTest, ThresholdNodeDifferentTypes)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    const std::vector<std::string> types = {
        "THRESH_BINARY", "THRESH_BINARY_INV", "THRESH_TRUNC", "THRESH_TOZERO", "THRESH_TOZERO_INV"
    };

    for (const auto &type : types)
    {
        node.SetInputSlotData("Input", inputImage);
        node.SetInputSlotData("Type", type);

        node.Process();

        auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
        ASSERT_TRUE(output.has_value()) << "Failed for type: " << type;
        EXPECT_FALSE(output->empty()) << "Failed for type: " << type;
    }
}

TEST_F(NodeImplementationTest, ThresholdNodeInvalidType)
{
    ThresholdNode node(1, "Threshold");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("Type", std::string("INVALID_TYPE"));

    node.Process();

    // Should default to THRESH_BINARY and not crash
    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

// ============================================================================
// CannyEdgeNode Tests
// ============================================================================

TEST_F(NodeImplementationTest, CannyEdgeNodeBasicProcessing)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("LowThreshold", 50.0);
    node.SetInputSlotData("HighThreshold", 150.0);
    node.SetInputSlotData("ApertureSize", 3);
    node.SetInputSlotData("L2Gradient", false);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
    EXPECT_EQ(output->rows, inputImage.rows);
    EXPECT_EQ(output->cols, inputImage.cols);
    EXPECT_EQ(output->type(), CV_8UC1);
}

TEST_F(NodeImplementationTest, CannyEdgeNodeWithDefaults)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    // Use defaults: LowThreshold=50, HighThreshold=150, ApertureSize=3, L2Gradient=false

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

TEST_F(NodeImplementationTest, CannyEdgeNodeColorImageConversion)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat colorImage = CreateTestImage();

    node.SetInputSlotData("Input", colorImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_EQ(output->type(), CV_8UC1);
}

TEST_F(NodeImplementationTest, CannyEdgeNodeNoInput)
{
    CannyEdgeNode node(1, "Canny");

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    EXPECT_FALSE(output.has_value() && !output->empty());
}

TEST_F(NodeImplementationTest, CannyEdgeNodeDifferentApertures)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    const std::vector<int> apertures = { 3, 5, 7 };

    for (int aperture : apertures)
    {
        node.SetInputSlotData("Input", inputImage);
        node.SetInputSlotData("ApertureSize", aperture);

        node.Process();

        auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
        ASSERT_TRUE(output.has_value()) << "Failed for aperture: " << aperture;
        EXPECT_FALSE(output->empty()) << "Failed for aperture: " << aperture;
    }
}

TEST_F(NodeImplementationTest, CannyEdgeNodeInvalidAperture)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("ApertureSize", 9); // Invalid, should clamp to 3

    node.Process();

    // Should default to 3 and not crash
    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

TEST_F(NodeImplementationTest, CannyEdgeNodeThresholdWarning)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("LowThreshold", 200.0); // Higher than high threshold
    node.SetInputSlotData("HighThreshold", 100.0);

    node.Process();

    // Should log warning but still process
    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

TEST_F(NodeImplementationTest, CannyEdgeNodeL2Gradient)
{
    CannyEdgeNode node(1, "Canny");
    cv::Mat inputImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", inputImage);
    node.SetInputSlotData("L2Gradient", true);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
}

// ============================================================================
// GrayscaleNode Tests
// ============================================================================

TEST_F(NodeImplementationTest, GrayscaleNodeColorToGray)
{
    GrayscaleNode node(1, "Grayscale");
    cv::Mat colorImage = CreateTestImage();

    node.SetInputSlotData("Input", colorImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
    EXPECT_EQ(output->rows, colorImage.rows);
    EXPECT_EQ(output->cols, colorImage.cols);
    EXPECT_EQ(output->channels(), 1);
    EXPECT_EQ(output->type(), CV_8UC1);
}

TEST_F(NodeImplementationTest, GrayscaleNodeAlreadyGray)
{
    GrayscaleNode node(1, "Grayscale");
    cv::Mat grayImage = CreateGrayscaleTestImage();

    node.SetInputSlotData("Input", grayImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    ASSERT_TRUE(output.has_value());
    EXPECT_FALSE(output->empty());
    EXPECT_EQ(output->channels(), 1);
}

TEST_F(NodeImplementationTest, GrayscaleNodeNoInput)
{
    GrayscaleNode node(1, "Grayscale");

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    EXPECT_FALSE(output.has_value() && !output->empty());
}

TEST_F(NodeImplementationTest, GrayscaleNodeEmptyImage)
{
    GrayscaleNode node(1, "Grayscale");
    cv::Mat emptyImage;

    node.SetInputSlotData("Input", emptyImage);

    node.Process();

    auto output = node.GetOutputSlot("Output").GetData<cv::Mat>();
    EXPECT_FALSE(output.has_value() && !output->empty());
}
