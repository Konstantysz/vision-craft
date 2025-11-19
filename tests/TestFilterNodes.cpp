#include "Vision/Algorithms/MedianBlurNode.h"
#include "Vision/Algorithms/MorphologyNode.h"
#include "Vision/Algorithms/SobelNode.h"
#include <opencv2/imgproc.hpp>
#include <gtest/gtest.h>

using namespace VisionCraft;

class TestFilterNodes : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a simple test image (checkerboard)
        inputImage = cv::Mat(10, 10, CV_8UC1, cv::Scalar(0));
        cv::rectangle(inputImage, cv::Rect(2, 2, 6, 6), cv::Scalar(255), -1);
    }

    cv::Mat inputImage;
};

TEST_F(TestFilterNodes, SobelNodeProcessing)
{
    auto node = std::make_unique<Vision::Algorithms::SobelNode>(1);

    // Set input
    node->SetInputSlotData("Input", inputImage);

    // Process
    node->Process();

    // Check output
    const auto &outputSlot = node->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputAny = outputSlot.GetVariantData();
    ASSERT_TRUE(std::holds_alternative<cv::Mat>(outputAny));

    cv::Mat outputImage = std::get<cv::Mat>(outputAny);
    EXPECT_FALSE(outputImage.empty());
    EXPECT_EQ(outputImage.size(), inputImage.size());
    EXPECT_EQ(outputImage.type(), inputImage.type());
}

TEST_F(TestFilterNodes, MedianBlurNodeProcessing)
{
    auto node = std::make_unique<Vision::Algorithms::MedianBlurNode>(1);

    // Set input
    node->SetInputSlotData("Input", inputImage);
    node->SetInputSlotData("ksize", 3);

    // Process
    node->Process();

    // Check output
    const auto &outputSlot = node->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputOpt = outputSlot.GetData<cv::Mat>();
    ASSERT_TRUE(outputOpt.has_value());
    cv::Mat outputImage = *outputOpt;

    EXPECT_FALSE(outputImage.empty());
    EXPECT_EQ(outputImage.size(), inputImage.size());
}

TEST_F(TestFilterNodes, MorphologyNodeProcessing)
{
    auto node = std::make_unique<Vision::Algorithms::MorphologyNode>(1);

    // Set input
    node->SetInputSlotData("Input", inputImage);
    node->SetInputSlotData("Operation", 1); // Dilate
    node->SetInputSlotData("ksize", 3);

    // Process
    node->Process();

    // Check output
    const auto &outputSlot = node->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputOpt = outputSlot.GetData<cv::Mat>();
    ASSERT_TRUE(outputOpt.has_value());
    cv::Mat outputImage = *outputOpt;

    EXPECT_FALSE(outputImage.empty());

    // Dilate should increase white area
    int inputWhitePixels = cv::countNonZero(inputImage);
    int outputWhitePixels = cv::countNonZero(outputImage);
    EXPECT_GT(outputWhitePixels, inputWhitePixels);
}
