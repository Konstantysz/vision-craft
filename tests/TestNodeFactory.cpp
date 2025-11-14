#include "Nodes/Factory/NodeFactory.h"
#include "Vision/Algorithms/CannyEdgeNode.h"
#include "Vision/Algorithms/GrayscaleNode.h"
#include "Vision/Algorithms/ThresholdNode.h"
#include "Vision/IO/ImageInputNode.h"
#include "Vision/IO/ImageOutputNode.h"
#include "Vision/IO/PreviewNode.h"

#include <gtest/gtest.h>
#include <memory>

using namespace VisionCraft;

class NodeFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Register all nodes before each test
        Nodes::NodeFactory::RegisterAllNodes();
    }
};

// ============================================================================
// Node Creation Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateImageInputNode)
{
    auto node = Nodes::NodeFactory::CreateNode("ImageInput", 1, "Test Input");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 1);
    EXPECT_EQ(node->GetName(), "Test Input");
    EXPECT_EQ(node->GetType(), "ImageInputNode");

    // Verify it's actually an ImageInputNode
    auto *inputNode = dynamic_cast<Vision::IO::ImageInputNode *>(node.get());
    EXPECT_NE(inputNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateImageOutputNode)
{
    auto node = Nodes::NodeFactory::CreateNode("ImageOutput", 2, "Test Output");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 2);
    EXPECT_EQ(node->GetName(), "Test Output");
    EXPECT_EQ(node->GetType(), "ImageOutputNode");

    auto *outputNode = dynamic_cast<Vision::IO::ImageOutputNode *>(node.get());
    EXPECT_NE(outputNode, nullptr);
}

TEST_F(NodeFactoryTest, CreatePreviewNode)
{
    auto node = Nodes::NodeFactory::CreateNode("Preview", 3, "Test Preview");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 3);
    EXPECT_EQ(node->GetName(), "Test Preview");
    EXPECT_EQ(node->GetType(), "PreviewNode");

    auto *previewNode = dynamic_cast<Vision::IO::PreviewNode *>(node.get());
    EXPECT_NE(previewNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateGrayscaleNode)
{
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", 4, "Test Grayscale");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 4);
    EXPECT_EQ(node->GetName(), "Test Grayscale");
    EXPECT_EQ(node->GetType(), "GrayscaleNode");

    auto *grayscaleNode = dynamic_cast<Vision::Algorithms::GrayscaleNode *>(node.get());
    EXPECT_NE(grayscaleNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateCannyEdgeNode)
{
    auto node = Nodes::NodeFactory::CreateNode("CannyEdge", 5, "Test Canny");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 5);
    EXPECT_EQ(node->GetName(), "Test Canny");
    EXPECT_EQ(node->GetType(), "CannyEdgeNode");

    auto *cannyNode = dynamic_cast<Vision::Algorithms::CannyEdgeNode *>(node.get());
    EXPECT_NE(cannyNode, nullptr);
}

TEST_F(NodeFactoryTest, CreateThresholdNode)
{
    auto node = Nodes::NodeFactory::CreateNode("Threshold", 6, "Test Threshold");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 6);
    EXPECT_EQ(node->GetName(), "Test Threshold");
    EXPECT_EQ(node->GetType(), "ThresholdNode");

    auto *thresholdNode = dynamic_cast<Vision::Algorithms::ThresholdNode *>(node.get());
    EXPECT_NE(thresholdNode, nullptr);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateUnknownNodeType)
{
    auto node = Nodes::NodeFactory::CreateNode("UnknownNodeType", 99, "Unknown");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithEmptyType)
{
    auto node = Nodes::NodeFactory::CreateNode("", 100, "Empty Type");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithInvalidType)
{
    auto node = Nodes::NodeFactory::CreateNode("InvalidNode", 101, "Invalid");

    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeFactoryTest, CreateWithWrongCaseType)
{
    // Type names are case-sensitive
    auto node = Nodes::NodeFactory::CreateNode("imageinputnode", 102, "Wrong Case");

    EXPECT_EQ(node, nullptr);
}

// ============================================================================
// Multiple Creation Tests
// ============================================================================

TEST_F(NodeFactoryTest, CreateMultipleNodesOfSameType)
{
    auto node1 = Nodes::NodeFactory::CreateNode("Grayscale", 1, "Gray1");
    auto node2 = Nodes::NodeFactory::CreateNode("Grayscale", 2, "Gray2");
    auto node3 = Nodes::NodeFactory::CreateNode("Grayscale", 3, "Gray3");

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
    auto input = Nodes::NodeFactory::CreateNode("ImageInput", 1, "Input");
    auto grayscale = Nodes::NodeFactory::CreateNode("Grayscale", 2, "Grayscale");
    auto threshold = Nodes::NodeFactory::CreateNode("Threshold", 3, "Threshold");
    auto canny = Nodes::NodeFactory::CreateNode("CannyEdge", 4, "Canny");
    auto preview = Nodes::NodeFactory::CreateNode("Preview", 5, "Preview");
    auto output = Nodes::NodeFactory::CreateNode("ImageOutput", 6, "Output");

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
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", 0, "Zero ID");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), 0);
}

TEST_F(NodeFactoryTest, CreateWithNegativeId)
{
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", -1, "Negative ID");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetId(), -1);
}

TEST_F(NodeFactoryTest, CreateWithEmptyName)
{
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", 1, "");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), "");
}

TEST_F(NodeFactoryTest, CreateWithLongName)
{
    std::string longName(1000, 'a');
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", 1, longName);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), longName);
}

TEST_F(NodeFactoryTest, CreateWithSpecialCharactersInName)
{
    std::string specialName = "Node-with_special.chars!@#$%";
    auto node = Nodes::NodeFactory::CreateNode("Grayscale", 1, specialName);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetName(), specialName);
}

// ============================================================================
// Node Uniqueness Tests
// ============================================================================

TEST_F(NodeFactoryTest, EachCreatedNodeIsUnique)
{
    auto node1 = Nodes::NodeFactory::CreateNode("Grayscale", 1, "Node1");
    auto node2 = Nodes::NodeFactory::CreateNode("Grayscale", 1, "Node1");

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
    auto thresholdNode = Nodes::NodeFactory::CreateNode("Threshold", 1, "Threshold");

    ASSERT_NE(thresholdNode, nullptr);

    // ThresholdNode should have specific input/output slots
    EXPECT_TRUE(thresholdNode->HasInputSlot("Input"));
    EXPECT_TRUE(thresholdNode->HasOutputSlot("Output"));
}

TEST_F(NodeFactoryTest, CreatedNodeCanProcess)
{
    auto grayscaleNode = Nodes::NodeFactory::CreateNode("Grayscale", 1, "Grayscale");

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
    const std::vector<std::pair<std::string, std::string>> nodeTypes = { { "ImageInput", "ImageInputNode" },
        { "ImageOutput", "ImageOutputNode" },
        { "Preview", "PreviewNode" },
        { "Grayscale", "GrayscaleNode" },
        { "CannyEdge", "CannyEdgeNode" },
        { "Threshold", "ThresholdNode" } };

    for (size_t i = 0; i < nodeTypes.size(); ++i)
    {
        const auto &[typeName, expectedType] = nodeTypes[i];
        auto node = Nodes::NodeFactory::CreateNode(typeName, static_cast<Nodes::NodeId>(i), "Test");

        ASSERT_NE(node, nullptr) << "Failed to create node type: " << typeName;
        EXPECT_EQ(node->GetType(), expectedType) << "Wrong type for: " << typeName;
    }
}
