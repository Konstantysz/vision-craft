#include "Vision/Algorithms/CvtColorNode.h"
#include "Vision/Algorithms/MergeChannelsNode.h"
#include "Vision/Algorithms/ResizeNode.h"
#include "Vision/Algorithms/SplitChannelsNode.h"
#include <opencv2/imgproc.hpp>
#include <gtest/gtest.h>

using namespace VisionCraft;

class TestConversionNodes : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a color test image
        inputImage = cv::Mat(10, 10, CV_8UC3, cv::Scalar(100, 150, 200));
    }

    cv::Mat inputImage;
};

TEST_F(TestConversionNodes, CvtColorNodeProcessing)
{
    auto node = std::make_unique<Vision::Algorithms::CvtColorNode>(1);

    node->SetInputSlotData("Input", inputImage);
    node->SetInputSlotData("Conversion", 0); // BGR2GRAY

    node->Process();

    const auto &outputSlot = node->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputOpt = outputSlot.GetData<cv::Mat>();
    ASSERT_TRUE(outputOpt.has_value());
    cv::Mat outputImage = *outputOpt;

    EXPECT_FALSE(outputImage.empty());
    EXPECT_EQ(outputImage.channels(), 1);
}

TEST_F(TestConversionNodes, ResizeNodeProcessing)
{
    auto node = std::make_unique<Vision::Algorithms::ResizeNode>(1);

    node->SetInputSlotData("Input", inputImage);
    node->SetInputSlotData("Width", 20);
    node->SetInputSlotData("Height", 20);

    node->Process();

    const auto &outputSlot = node->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputOpt = outputSlot.GetData<cv::Mat>();
    ASSERT_TRUE(outputOpt.has_value());
    cv::Mat outputImage = *outputOpt;

    EXPECT_EQ(outputImage.cols, 20);
    EXPECT_EQ(outputImage.rows, 20);
}

TEST_F(TestConversionNodes, SplitMergeNodesProcessing)
{
    // Test Split
    auto splitNode = std::make_unique<Vision::Algorithms::SplitChannelsNode>(1);
    splitNode->SetInputSlotData("Input", inputImage);
    splitNode->Process();

    auto c1Opt = splitNode->GetOutputSlot("Channel 1").GetData<cv::Mat>();
    auto c2Opt = splitNode->GetOutputSlot("Channel 2").GetData<cv::Mat>();
    auto c3Opt = splitNode->GetOutputSlot("Channel 3").GetData<cv::Mat>();

    ASSERT_TRUE(c1Opt.has_value());
    ASSERT_TRUE(c2Opt.has_value());
    ASSERT_TRUE(c3Opt.has_value());

    // Test Merge
    auto mergeNode = std::make_unique<Vision::Algorithms::MergeChannelsNode>(2);
    mergeNode->SetInputSlotData("Channel 1", *c1Opt);
    mergeNode->SetInputSlotData("Channel 2", *c2Opt);
    mergeNode->SetInputSlotData("Channel 3", *c3Opt);

    mergeNode->Process();

    const auto &outputSlot = mergeNode->GetOutputSlot("Output");
    ASSERT_TRUE(outputSlot.HasData());

    auto outputOpt = outputSlot.GetData<cv::Mat>();
    ASSERT_TRUE(outputOpt.has_value());
    cv::Mat outputImage = *outputOpt;

    EXPECT_EQ(outputImage.channels(), 3);
    EXPECT_EQ(outputImage.size(), inputImage.size());
}
