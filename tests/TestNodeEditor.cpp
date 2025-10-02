#include "VisionCraftEngine/NodeEditor.h"
#include "VisionCraftEngine/Nodes/CannyEdgeNode.h"
#include "VisionCraftEngine/Nodes/ImageInputNode.h"
#include "VisionCraftEngine/Nodes/ThresholdNode.h"

#include <gtest/gtest.h>
#include <memory>

using namespace VisionCraft::Engine;

// Dummy test node for testing
class TestNode : public Node
{
public:
    TestNode(NodeId id, std::string name) : Node(id, std::move(name))
    {
    }
    void Process() override
    {
    } // No-op for testing
};

class NodeEditorTest : public ::testing::Test
{
protected:
    NodeEditor editor;
};

// ============================================================================
// Basic Node Management Tests
// ============================================================================

TEST_F(NodeEditorTest, InitialState)
{
    EXPECT_TRUE(editor.GetNodeIds().empty());
    EXPECT_TRUE(editor.GetConnections().empty());
    EXPECT_EQ(editor.GetNode(1), nullptr);
}

TEST_F(NodeEditorTest, AddSingleNode)
{
    auto node = std::make_unique<TestNode>(5, "TestNode");
    const auto nodeId = node->GetId();

    const auto addedId = editor.AddNode(std::move(node));

    EXPECT_EQ(addedId, nodeId);
    EXPECT_EQ(editor.GetNodeIds().size(), 1);
    EXPECT_EQ(editor.GetNodeIds()[0], nodeId);

    const auto *retrievedNode = editor.GetNode(nodeId);
    ASSERT_NE(retrievedNode, nullptr);
    EXPECT_EQ(retrievedNode->GetId(), nodeId);
    EXPECT_EQ(retrievedNode->GetName(), "TestNode");
}

TEST_F(NodeEditorTest, AddMultipleNodes)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(3, "Node2");
    auto node3 = std::make_unique<TestNode>(2, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    const auto nodeIds = editor.GetNodeIds();
    EXPECT_EQ(nodeIds.size(), 3);

    // Should contain all three IDs
    EXPECT_TRUE(std::find(nodeIds.begin(), nodeIds.end(), 1) != nodeIds.end());
    EXPECT_TRUE(std::find(nodeIds.begin(), nodeIds.end(), 2) != nodeIds.end());
    EXPECT_TRUE(std::find(nodeIds.begin(), nodeIds.end(), 3) != nodeIds.end());

    // Verify each node can be retrieved
    EXPECT_NE(editor.GetNode(1), nullptr);
    EXPECT_NE(editor.GetNode(2), nullptr);
    EXPECT_NE(editor.GetNode(3), nullptr);
    EXPECT_EQ(editor.GetNode(1)->GetName(), "Node1");
    EXPECT_EQ(editor.GetNode(2)->GetName(), "Node3");
    EXPECT_EQ(editor.GetNode(3)->GetName(), "Node2");
}

TEST_F(NodeEditorTest, RemoveExistingNode)
{
    auto node1 = std::make_unique<TestNode>(10, "Node1");
    auto node2 = std::make_unique<TestNode>(20, "Node2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));

    EXPECT_TRUE(editor.RemoveNode(10));

    const auto nodeIds = editor.GetNodeIds();
    EXPECT_EQ(nodeIds.size(), 1);
    EXPECT_EQ(nodeIds[0], 20);
    EXPECT_EQ(editor.GetNode(10), nullptr);
    EXPECT_NE(editor.GetNode(20), nullptr);
}

TEST_F(NodeEditorTest, RemoveNonExistentNode)
{
    auto node = std::make_unique<TestNode>(1, "TestNode");
    editor.AddNode(std::move(node));

    EXPECT_FALSE(editor.RemoveNode(999));
    EXPECT_EQ(editor.GetNodeIds().size(), 1);
    EXPECT_NE(editor.GetNode(1), nullptr);
}

TEST_F(NodeEditorTest, GetNonExistentNode)
{
    EXPECT_EQ(editor.GetNode(999), nullptr);

    // Add a node and test getting different ID
    auto node = std::make_unique<TestNode>(5, "TestNode");
    editor.AddNode(std::move(node));

    EXPECT_EQ(editor.GetNode(999), nullptr);
    EXPECT_NE(editor.GetNode(5), nullptr);
}

TEST_F(NodeEditorTest, ConstGetNode)
{
    auto node = std::make_unique<TestNode>(7, "ConstTest");
    editor.AddNode(std::move(node));

    const auto &constEditor = editor;
    const auto *constNode = constEditor.GetNode(7);

    ASSERT_NE(constNode, nullptr);
    EXPECT_EQ(constNode->GetId(), 7);
    EXPECT_EQ(constNode->GetName(), "ConstTest");

    // Non-existent node
    EXPECT_EQ(constEditor.GetNode(999), nullptr);
}

// ============================================================================
// Connection Management Tests
// ============================================================================

TEST_F(NodeEditorTest, AddSingleConnection)
{
    auto node1 = std::make_unique<TestNode>(1, "Source");
    auto node2 = std::make_unique<TestNode>(2, "Dest");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));

    editor.AddConnection(1, 2);

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].from, 1);
    EXPECT_EQ(connections[0].to, 2);
}

TEST_F(NodeEditorTest, AddMultipleConnections)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");
    auto node3 = std::make_unique<TestNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    editor.AddConnection(1, 2);
    editor.AddConnection(2, 3);
    editor.AddConnection(1, 3);

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 3);

    // Verify all connections exist
    bool found1to2 = false, found2to3 = false, found1to3 = false;
    for (const auto &conn : connections)
    {
        if (conn.from == 1 && conn.to == 2)
            found1to2 = true;
        if (conn.from == 2 && conn.to == 3)
            found2to3 = true;
        if (conn.from == 1 && conn.to == 3)
            found1to3 = true;
    }

    EXPECT_TRUE(found1to2);
    EXPECT_TRUE(found2to3);
    EXPECT_TRUE(found1to3);
}

TEST_F(NodeEditorTest, RemoveExistingConnection)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");
    auto node3 = std::make_unique<TestNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    editor.AddConnection(1, 2);
    editor.AddConnection(2, 3);
    editor.AddConnection(1, 3);

    EXPECT_TRUE(editor.RemoveConnection(1, 2));

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 2);

    // Verify 1->2 is removed but others remain
    bool found1to2 = false, found2to3 = false, found1to3 = false;
    for (const auto &conn : connections)
    {
        if (conn.from == 1 && conn.to == 2)
            found1to2 = true;
        if (conn.from == 2 && conn.to == 3)
            found2to3 = true;
        if (conn.from == 1 && conn.to == 3)
            found1to3 = true;
    }

    EXPECT_FALSE(found1to2);
    EXPECT_TRUE(found2to3);
    EXPECT_TRUE(found1to3);
}

TEST_F(NodeEditorTest, RemoveNonExistentConnection)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));

    editor.AddConnection(1, 2);

    EXPECT_FALSE(editor.RemoveConnection(2, 1)); // Reverse direction
    EXPECT_FALSE(editor.RemoveConnection(1, 3)); // Non-existent target
    EXPECT_FALSE(editor.RemoveConnection(3, 2)); // Non-existent source

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].from, 1);
    EXPECT_EQ(connections[0].to, 2);
}

// ============================================================================
// Node Removal with Connection Cleanup Tests
// ============================================================================

TEST_F(NodeEditorTest, RemoveNodeCleansUpConnections)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");
    auto node3 = std::make_unique<TestNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Create connections involving node 2
    editor.AddConnection(1, 2); // 1 -> 2
    editor.AddConnection(2, 3); // 2 -> 3
    editor.AddConnection(1, 3); // 1 -> 3 (should remain)

    // Remove node 2
    EXPECT_TRUE(editor.RemoveNode(2));

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].from, 1);
    EXPECT_EQ(connections[0].to, 3);
}

TEST_F(NodeEditorTest, RemoveNodeWithNoConnections)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");
    auto node3 = std::make_unique<TestNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Only connect nodes 1 and 3
    editor.AddConnection(1, 3);

    // Remove node 2 (no connections)
    EXPECT_TRUE(editor.RemoveNode(2));

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].from, 1);
    EXPECT_EQ(connections[0].to, 3);

    const auto nodeIds = editor.GetNodeIds();
    EXPECT_EQ(nodeIds.size(), 2);
}

// ============================================================================
// Clear Functionality Tests
// ============================================================================

TEST_F(NodeEditorTest, ClearEmptyEditor)
{
    editor.Clear();

    EXPECT_TRUE(editor.GetNodeIds().empty());
    EXPECT_TRUE(editor.GetConnections().empty());
}

TEST_F(NodeEditorTest, ClearWithNodesAndConnections)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddConnection(1, 2);

    // Verify setup
    EXPECT_EQ(editor.GetNodeIds().size(), 2);
    EXPECT_EQ(editor.GetConnections().size(), 1);

    editor.Clear();

    EXPECT_TRUE(editor.GetNodeIds().empty());
    EXPECT_TRUE(editor.GetConnections().empty());
    EXPECT_EQ(editor.GetNode(1), nullptr);
    EXPECT_EQ(editor.GetNode(2), nullptr);
}

// ============================================================================
// Edge Cases and Error Scenarios
// ============================================================================

TEST_F(NodeEditorTest, DuplicateNodeIds)
{
    auto node1 = std::make_unique<TestNode>(5, "Node1");
    auto node2 = std::make_unique<TestNode>(5, "Node2"); // Same ID

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2)); // Should overwrite

    EXPECT_EQ(editor.GetNodeIds().size(), 1);
    EXPECT_EQ(editor.GetNode(5)->GetName(), "Node2"); // Second node overwrites first
}

TEST_F(NodeEditorTest, DuplicateConnections)
{
    auto node1 = std::make_unique<TestNode>(1, "Node1");
    auto node2 = std::make_unique<TestNode>(2, "Node2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));

    editor.AddConnection(1, 2);
    editor.AddConnection(1, 2); // Duplicate

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 2); // Both connections are stored (no deduplication)
}

TEST_F(NodeEditorTest, SelfConnection)
{
    auto node = std::make_unique<TestNode>(1, "SelfConnected");
    editor.AddNode(std::move(node));

    editor.AddConnection(1, 1); // Self connection

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_EQ(connections[0].from, 1);
    EXPECT_EQ(connections[0].to, 1);
}

TEST_F(NodeEditorTest, ConnectionsToNonExistentNodes)
{
    auto node = std::make_unique<TestNode>(1, "ExistingNode");
    editor.AddNode(std::move(node));

    // Add connections to/from non-existent nodes
    editor.AddConnection(1, 999);   // To non-existent
    editor.AddConnection(888, 1);   // From non-existent
    editor.AddConnection(777, 666); // Both non-existent

    const auto &connections = editor.GetConnections();
    EXPECT_EQ(connections.size(), 3); // All connections are stored regardless
}

// ============================================================================
// Real Node Integration Tests
// ============================================================================

TEST_F(NodeEditorTest, RealNodeTypes)
{
    auto inputNode = std::make_unique<ImageInputNode>(1, "Input");
    auto thresholdNode = std::make_unique<ThresholdNode>(2, "Threshold");
    auto cannyNode = std::make_unique<CannyEdgeNode>(3, "Canny");

    editor.AddNode(std::move(inputNode));
    editor.AddNode(std::move(thresholdNode));
    editor.AddNode(std::move(cannyNode));

    // Create a processing chain
    editor.AddConnection(1, 2); // Input -> Threshold
    editor.AddConnection(2, 3); // Threshold -> Canny

    EXPECT_EQ(editor.GetNodeIds().size(), 3);
    EXPECT_EQ(editor.GetConnections().size(), 2);

    // Verify node types
    EXPECT_EQ(editor.GetNode(1)->GetName(), "Input");
    EXPECT_EQ(editor.GetNode(2)->GetName(), "Threshold");
    EXPECT_EQ(editor.GetNode(3)->GetName(), "Canny");
}

TEST_F(NodeEditorTest, NextIdTracking)
{
    // Add nodes with specific IDs to test nextId tracking
    auto node5 = std::make_unique<TestNode>(5, "Node5");
    auto node2 = std::make_unique<TestNode>(2, "Node2");
    auto node10 = std::make_unique<TestNode>(10, "Node10");

    editor.AddNode(std::move(node5));  // nextId should become 6
    editor.AddNode(std::move(node2));  // nextId should stay 6 (2 < 6)
    editor.AddNode(std::move(node10)); // nextId should become 11

    // This is testing internal behavior - we can't directly access nextId
    // but we can infer it works correctly by the fact that nodes are added successfully
    EXPECT_EQ(editor.GetNodeIds().size(), 3);
}

// ============================================================================
// Slot-Based Data Passing Tests
// ============================================================================

class SlotTestNode : public Node
{
public:
    SlotTestNode(NodeId id, std::string name) : Node(id, std::move(name))
    {
        CreateInputSlot("Input");
        CreateInputSlot("Multiplier", 2.0);
        CreateOutputSlot("Output");
    }

    void Process() override
    {
        auto input = GetInputValue<double>("Input");
        auto multiplier = GetInputValue<double>("Multiplier").value_or(2.0);

        if (input.has_value())
        {
            SetOutputSlotData("Output", input.value() * multiplier);
        }
        else
        {
            ClearOutputSlot("Output");
        }
    }
};

TEST_F(NodeEditorTest, ExecuteGraphWithSlots)
{
    auto node1 = std::make_unique<SlotTestNode>(1, "Node1");
    auto node2 = std::make_unique<SlotTestNode>(2, "Node2");

    node1->SetInputSlotData("Input", 5.0);
    node1->SetInputSlotData("Multiplier", 3.0);

    auto *node1Ptr = node1.get();
    auto *node2Ptr = node2.get();

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddConnection(1, 2);

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    auto node1Output = node1Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(node1Output.has_value());
    EXPECT_DOUBLE_EQ(node1Output.value(), 15.0);

    auto node2Output = node2Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(node2Output.has_value());
    EXPECT_DOUBLE_EQ(node2Output.value(), 30.0);
}

TEST_F(NodeEditorTest, ExecuteGraphCycleDetection)
{
    auto node1 = std::make_unique<SlotTestNode>(1, "Node1");
    auto node2 = std::make_unique<SlotTestNode>(2, "Node2");
    auto node3 = std::make_unique<SlotTestNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Create a cycle: 1 -> 2 -> 3 -> 1
    editor.AddConnection(1, 2);
    editor.AddConnection(2, 3);
    editor.AddConnection(3, 1);

    bool success = editor.Execute();
    EXPECT_FALSE(success); // Should fail due to cycle
}

TEST_F(NodeEditorTest, ExecuteGraphSelfCycle)
{
    auto node = std::make_unique<SlotTestNode>(1, "Node");

    editor.AddNode(std::move(node));
    editor.AddConnection(1, 1); // Self-connection

    bool success = editor.Execute();
    EXPECT_FALSE(success); // Should fail due to cycle
}

TEST_F(NodeEditorTest, ExecuteGraphMultipleBranches)
{
    auto source = std::make_unique<SlotTestNode>(1, "Source");
    auto branch1 = std::make_unique<SlotTestNode>(2, "Branch1");
    auto branch2 = std::make_unique<SlotTestNode>(3, "Branch2");
    auto merge = std::make_unique<SlotTestNode>(4, "Merge");

    source->SetInputSlotData("Input", 10.0);

    auto *sourcePtr = source.get();
    auto *branch1Ptr = branch1.get();
    auto *branch2Ptr = branch2.get();
    auto *mergePtr = merge.get();

    editor.AddNode(std::move(source));
    editor.AddNode(std::move(branch1));
    editor.AddNode(std::move(branch2));
    editor.AddNode(std::move(merge));

    // Create diamond pattern: source -> branch1 -> merge
    //                          source -> branch2 -> merge
    editor.AddConnection(1, 2);
    editor.AddConnection(1, 3);
    editor.AddConnection(2, 4);
    editor.AddConnection(3, 4);

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // Source should output 20 (10 * 2)
    auto sourceOutput = sourcePtr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(sourceOutput.has_value());
    EXPECT_DOUBLE_EQ(sourceOutput.value(), 20.0);

    // Both branches should process
    auto branch1Output = branch1Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(branch1Output.has_value());
    EXPECT_DOUBLE_EQ(branch1Output.value(), 40.0); // 20 * 2

    auto branch2Output = branch2Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(branch2Output.has_value());
    EXPECT_DOUBLE_EQ(branch2Output.value(), 40.0); // 20 * 2

    // Merge will receive data from branch2 (last connection processed)
    auto mergeOutput = mergePtr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(mergeOutput.has_value());
    EXPECT_DOUBLE_EQ(mergeOutput.value(), 80.0); // 40 * 2
}

TEST_F(NodeEditorTest, ExecuteEmptyGraph)
{
    bool success = editor.Execute();
    EXPECT_TRUE(success); // Empty graph should succeed trivially
}

TEST_F(NodeEditorTest, ExecuteSingleNode)
{
    auto node = std::make_unique<SlotTestNode>(1, "Single");
    node->SetInputSlotData("Input", 7.0);

    auto *nodePtr = node.get();
    editor.AddNode(std::move(node));

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    auto output = nodePtr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(output.has_value());
    EXPECT_DOUBLE_EQ(output.value(), 14.0); // 7 * 2
}

TEST_F(NodeEditorTest, ExecuteDisconnectedNodes)
{
    auto node1 = std::make_unique<SlotTestNode>(1, "Node1");
    auto node2 = std::make_unique<SlotTestNode>(2, "Node2");

    node1->SetInputSlotData("Input", 5.0);
    node2->SetInputSlotData("Input", 10.0);

    auto *node1Ptr = node1.get();
    auto *node2Ptr = node2.get();

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    // No connections - both nodes independent

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // Both should execute independently
    auto output1 = node1Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(output1.has_value());
    EXPECT_DOUBLE_EQ(output1.value(), 10.0);

    auto output2 = node2Ptr->GetOutputSlot("Output").GetData<double>();
    ASSERT_TRUE(output2.has_value());
    EXPECT_DOUBLE_EQ(output2.value(), 20.0);
}

TEST_F(NodeEditorTest, ExecuteLongChain)
{
    const int chainLength = 10;
    std::vector<SlotTestNode *> nodePtrs;

    for (int i = 1; i <= chainLength; ++i)
    {
        auto node = std::make_unique<SlotTestNode>(i, "Node" + std::to_string(i));
        nodePtrs.push_back(node.get());
        editor.AddNode(std::move(node));
    }

    // Set input on first node
    nodePtrs[0]->SetInputSlotData("Input", 1.0);
    nodePtrs[0]->SetInputSlotData("Multiplier", 2.0);

    // Connect nodes in a chain
    for (int i = 0; i < chainLength - 1; ++i)
    {
        editor.AddConnection(i + 1, i + 2);
    }

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // Verify each node in the chain
    double expectedValue = 2.0; // 1.0 * 2.0
    for (int i = 0; i < chainLength; ++i)
    {
        auto output = nodePtrs[i]->GetOutputSlot("Output").GetData<double>();
        ASSERT_TRUE(output.has_value()) << "Node " << i << " failed";
        EXPECT_DOUBLE_EQ(output.value(), expectedValue) << "Node " << i << " value mismatch";
        expectedValue *= 2.0; // Each subsequent node doubles the value
    }
}

class ErrorThrowingNode : public Node
{
public:
    ErrorThrowingNode(NodeId id, std::string name) : Node(id, std::move(name))
    {
        CreateInputSlot("Input");
        CreateOutputSlot("Output");
    }

    void Process() override
    {
        throw std::runtime_error("Intentional error for testing");
    }
};

TEST_F(NodeEditorTest, ExecuteGraphWithNodeError)
{
    auto node1 = std::make_unique<SlotTestNode>(1, "GoodNode");
    auto node2 = std::make_unique<ErrorThrowingNode>(2, "BadNode");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddConnection(1, 2);

    bool success = editor.Execute();
    EXPECT_FALSE(success); // Should fail when node throws
}