#include "VisionCraftEngine/NodeFactory.h"
#include "VisionCraftEngine/Nodes/CannyEdgeNode.h"
#include "VisionCraftEngine/Nodes/GrayscaleNode.h"
#include "VisionCraftEngine/Nodes/ImageInputNode.h"
#include "VisionCraftEngine/Nodes/ImageOutputNode.h"
#include "VisionCraftEngine/Nodes/PreviewNode.h"
#include "VisionCraftEngine/Nodes/ThresholdNode.h"

#include <gtest/gtest.h>
#include <memory>

using namespace VisionCraft::Engine;

class NodeFactoryTest : public ::testing::Test
{
protected:
};

// ============================================================================
// Node Creation Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateImageInputNode)
{
    auto node = NodeFactory::CreateNode("ImageInputNode", 1, "Test Input");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 1);
    EXPECT_EQ(node->GetName(), "Test Input");
    EXPECT_EQ(node->GetType(), "ImageInputNode");

    // Verify it's actually an ImageInputNode
    auto *inputNode = dynamic_cast<ImageInputNode *>(node.get());
    EXPECT_NE(inputNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateImageOutputNode)
{
    auto node = NodeFactory::CreateNode("ImageOutputNode", 2, "Test Output");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 2);
    EXPECT_EQ(node->GetName(), "Test Output");
    EXPECT_EQ(node->GetType(), "ImageOutputNode");

    auto *outputNode = dynamic_cast<ImageOutputNode *>(node.get());
    EXPECT_NE(outputNode, nullptr);
}

TEST_F(NodeFactoryTest, CreatePreviewNode)
{
    auto node = NodeFactory::CreateNode("PreviewNode", 3, "Test Preview");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 3);
    EXPECT_EQ(node->GetName(), "Test Preview");
    EXPECT_EQ(node->GetType(), "PreviewNode");

    auto *previewNode = dynamic_cast<PreviewNode *>(node.get());
    EXPECT_NE(previewNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateGrayscaleNode)
{
    auto node = NodeFactory::CreateNode("GrayscaleNode", 4, "Test Grayscale");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 4);
    EXPECT_EQ(node->GetName(), "Test Grayscale");
    EXPECT_EQ(node->GetType(), "GrayscaleNode");

    auto *grayscaleNode = dynamic_cast<GrayscaleNode *>(node.get());
    EXPECT_NE(grayscaleNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateCannyEdgeNode)
{
    auto node = NodeFactory::CreateNode("CannyEdgeNode", 5, "Test Canny");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 5);
    EXPECT_EQ(node->GetName(), "Test Canny");
    EXPECT_EQ(node->GetType(), "CannyEdgeNode");

    auto *cannyNode = dynamic_cast<CannyEdgeNode *>(node.get());
    EXPECT_NE(cannyNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateThresholdNode)
{
    auto node = NodeFactory::CreateNode("ThresholdNode", 6, "Test Threshold");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 6);
    EXPECT_EQ(node->GetName(), "Test Threshold");
    EXPECT_EQ(node->GetType(), "ThresholdNode");

    auto *thresholdNode = dynamic_cast<ThresholdNode *>(node.get());
    EXPECT_NE(thresholdNode, nullptr);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateUnknownNodeType)
{
    auto node = NodeFactory::CreateNode("UnknownNodeType", 99, "Unknown");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithEmptyType)
{
    auto node = NodeFactory::CreateNode("", 100, "Empty Type");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithInvalidType)
{
    auto node = NodeFactory::CreateNode("InvalidNode", 101, "Invalid");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithWrongCaseType)
{
    // Type names are case-sensitive
    auto node = NodeFactory::CreateNode("imageinputnode", 102, "Wrong Case");

    EXPECT_EQ(node, nullptr);
}

// ============================================================================
// Multiple Creation Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateMultipleNodesOfSameType)
{
    auto node1 = NodeFactory::CreateNode("GrayscaleNode", 1, "Gray1");
    auto node2 = NodeFactory::CreateNode("GrayscaleNode", 2, "Gray2");
    auto node3 = NodeFactory::CreateNode("GrayscaleNode", 3, "Gray3");

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    ASSERT_NE(node3, nullptr);

    EXPECT_EQ(node1->GetId(), 1);
    EXPECT_EQ(node2->GetId(), 2);
    EXPECT_EQ(node3->GetId(), 3);

    EXPECT_EQ(node1->GetName(), "Gray1");
    EXPECT_EQ(node2->GetName(), "Gray2");
    EXPECT_EQ(node3->GetName(), "Gray3");
}

TEST_F(NodeFactoryTest, CreateMultipleNodesOfDifferentTypes)
{
    auto input = NodeFactory::CreateNode("ImageInputNode", 1, "Input");
    auto grayscale = NodeFactory::CreateNode("GrayscaleNode", 2, "Grayscale");
    auto threshold = NodeFactory::CreateNode("ThresholdNode", 3, "Threshold");
    auto canny = NodeFactory::CreateNode("CannyEdgeNode", 4, "Canny");
    auto preview = NodeFactory::CreateNode("PreviewNode", 5, "Preview");
    auto output = NodeFactory::CreateNode("ImageOutputNode", 6, "Output");

    ASSERT_NE(input, nullptr);
    ASSERT_NE(grayscale, nullptr);
    ASSERT_NE(threshold, nullptr);
    ASSERT_NE(canny, nullptr);
    ASSERT_NE(preview, nullptr);
    ASSERT_NE(output, nullptr);

    EXPECT_EQ(input->GetType(), "ImageInputNode");
    EXPECT_EQ(grayscale->GetType(), "GrayscaleNode");
    EXPECT_EQ(threshold->GetType(), "ThresholdNode");
    EXPECT_EQ(canny->GetType(), "CannyEdgeNode");
    EXPECT_EQ(preview->GetType(), "PreviewNode");
    EXPECT_EQ(output->GetType(), "ImageOutputNode");
}

// ============================================================================
// Node ID and Name Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateWithZeroId)
{
    auto node = NodeFactory::CreateNode("GrayscaleNode", 0, "Zero ID");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 0);
}

TEST_F(NodeFactoryTest, CreateWithNegativeId)
{
    auto node = NodeFactory::CreateNode("GrayscaleNode", -1, "Negative ID");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), -1);
}

TEST_F(NodeFactoryTest, CreateWithEmptyName)
{
    auto node = NodeFactory::CreateNode("GrayscaleNode", 1, "");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), "");
}

TEST_F(NodeFactoryTest, CreateWithLongName)
{
    std::string longName(1000, 'a');
    auto node = NodeFactory::CreateNode("GrayscaleNode", 1, longName);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), longName);
}

TEST_F(NodeFactoryTest, CreateWithSpecialCharactersInName)
{
    std::string specialName = "Node-with_special.chars!@#$%";
    auto node = NodeFactory::CreateNode("GrayscaleNode", 1, specialName);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), specialName);
}

// ============================================================================
// Node Uniqueness Tests
// ============================================================================

TEST_F(NodeFactoryTest, EachCreatedNodeIsUnique)
{
    auto node1 = NodeFactory::CreateNode("GrayscaleNode", 1, "Node1");
    auto node2 = NodeFactory::CreateNode("GrayscaleNode", 1, "Node1");

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);

    // Same ID and name, but different objects
    EXPECT_NE(node1.get(), node2.get());
}

// ============================================================================
// Node Functionality After Creation Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreatedNodeHasCorrectSlots)
{
    auto thresholdNode = NodeFactory::CreateNode("ThresholdNode", 1, "Threshold");

    ASSERT_NE(thresholdNode, nullptr);

    // ThresholdNode should have specific input/output slots
    EXPECT_TRUE(thresholdNode->HasInputSlot("Input"));
    EXPECT_TRUE(thresholdNode->HasOutputSlot("Output"));
}

TEST_F(NodeFactoryTest, CreatedNodeCanProcess)
{
    auto grayscaleNode = NodeFactory::CreateNode("GrayscaleNode", 1, "Grayscale");

    ASSERT_NE(grayscaleNode, nullptr);

    // Create a test image
    cv::Mat testImage = cv::Mat::zeros(100, 100, CV_8UC3);
    grayscaleNode->SetInputSlotData("Input", testImage);

    // Should not throw
    EXPECT_NO_THROW(grayscaleNode->Process());
}

// ============================================================================
// All Node Types Coverage Test
// ============================================================================

TEST_F(NodeFactoryTest, CreateAllAvailableNodeTypes)
{
    const std::vector<std::pair<std::string, std::string>> nodeTypes = { { "ImageInputNode", "ImageInputNode" },
        { "ImageOutputNode", "ImageOutputNode" },
        { "PreviewNode", "PreviewNode" },
        { "GrayscaleNode", "GrayscaleNode" },
        { "CannyEdgeNode", "CannyEdgeNode" },
        { "ThresholdNode", "ThresholdNode" } };

    for (size_t i = 0; i < nodeTypes.size(); ++i)
    {
        const auto &[typeName, expectedType] = nodeTypes[i];
        auto node = NodeFactory::CreateNode(typeName, static_cast<NodeId>(i), "Test");

        ASSERT_NE(node, nullptr) << "Failed to create node type: " << typeName;
        EXPECT_EQ(node->GetType(), expectedType) << "Wrong type for: " << typeName;
    }
}
