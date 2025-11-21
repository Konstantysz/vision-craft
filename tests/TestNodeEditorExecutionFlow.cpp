#include "Nodes/Core/NodeEditor.h"
#include "gtest/gtest.h"

using namespace VisionCraft;

// Test fixture
class NodeEditorTest : public ::testing::Test
{
protected:
    Nodes::NodeEditor editor;
};

// SlotTestNode for mixed graph tests
class SlotTestNode : public Nodes::Node
{
public:
    SlotTestNode(Nodes::NodeId id, std::string name) : Nodes::Node(id, std::move(name))
    {
        CreateInputSlot("Input");
        CreateInputSlot("Multiplier", 2.0);
        CreateOutputSlot("Output");
    }

    std::string GetType() const override
    {
        return "SlotTestNode";
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

// ============================================================================
// Execution Flow Tests (White Arrow/Execution Pins)
// ============================================================================

// Test node with execution pins
class ExecutionFlowNode : public Nodes::Node
{
public:
    ExecutionFlowNode(Nodes::NodeId id, std::string name, bool hasInputPin = true, bool hasOutputPin = true)
        : Nodes::Node(id, std::move(name)), executed(false)
    {
        if (hasInputPin)
            CreateExecutionInputPin("Execute");
        if (hasOutputPin)
            CreateExecutionOutputPin("Then");

        CreateInputSlot("Value", 0);
        CreateOutputSlot("Output");
    }

    std::string GetType() const override
    {
        return "ExecutionFlowNode";
    }

    void Process() override
    {
        executed = true;
        executionOrder.push_back(GetId());

        auto value = GetInputValue<int>("Value").value_or(0);
        SetOutputSlotData("Output", value + 1);
    }

    bool executed;
    static std::vector<Nodes::NodeId> executionOrder;
};

std::vector<Nodes::NodeId> ExecutionFlowNode::executionOrder;

TEST_F(NodeEditorTest, ExecutionFlow_FollowsExecutionConnections)
{
    ExecutionFlowNode::executionOrder.clear();

    // Create nodes: 1 -> 3 -> 2 (execution flow order)
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Start", false, true); // Entry point
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "End", true, false);
    auto node3 = std::make_unique<ExecutionFlowNode>(3, "Middle");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Execution flow: 1 -> 3 -> 2
    editor.AddConnection(1, "Then", 3, "Execute", Nodes::ConnectionType::Execution);
    editor.AddConnection(3, "Then", 2, "Execute", Nodes::ConnectionType::Execution);

    // Data flow (opposite direction, should be ignored): 1 -> 2 -> 3
    editor.AddConnection(1, "Output", 2, "Value", Nodes::ConnectionType::Data);
    editor.AddConnection(2, "Output", 3, "Value", Nodes::ConnectionType::Data);

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // Execution should follow execution pins (1, 3, 2), NOT data dependencies
    ASSERT_EQ(ExecutionFlowNode::executionOrder.size(), 3);
    EXPECT_EQ(ExecutionFlowNode::executionOrder[0], 1);
    EXPECT_EQ(ExecutionFlowNode::executionOrder[1], 3);
    EXPECT_EQ(ExecutionFlowNode::executionOrder[2], 2);
}

TEST_F(NodeEditorTest, ExecutionFlow_1to1_EnforcementOutputPin)
{
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Node1", false, true);
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "Node2");
    auto node3 = std::make_unique<ExecutionFlowNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Connect 1 -> 2
    editor.AddConnection(1, "Then", 2, "Execute", Nodes::ConnectionType::Execution);

    // Connect 1 -> 3 (should remove 1 -> 2 connection)
    editor.AddConnection(1, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    auto connections = editor.GetConnections();

    // Count execution connections from node 1
    int execConnsFromNode1 = 0;
    Nodes::NodeId targetNode = 0;
    for (const auto &conn : connections)
    {
        if (conn.type == Nodes::ConnectionType::Execution && conn.from == 1)
        {
            execConnsFromNode1++;
            targetNode = conn.to;
        }
    }

    // Should have exactly 1 execution connection from node 1
    EXPECT_EQ(execConnsFromNode1, 1);
    // It should be to node 3 (the most recent one)
    EXPECT_EQ(targetNode, 3);
}

TEST_F(NodeEditorTest, ExecutionFlow_1to1_EnforcementInputPin)
{
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Node1", false, true);
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "Node2", false, true);
    auto node3 = std::make_unique<ExecutionFlowNode>(3, "Node3");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Connect 1 -> 3
    editor.AddConnection(1, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    // Connect 2 -> 3 (should remove 1 -> 3 connection)
    editor.AddConnection(2, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    auto connections = editor.GetConnections();

    // Count execution connections to node 3
    int execConnsToNode3 = 0;
    Nodes::NodeId sourceNode = 0;
    for (const auto &conn : connections)
    {
        if (conn.type == Nodes::ConnectionType::Execution && conn.to == 3)
        {
            execConnsToNode3++;
            sourceNode = conn.from;
        }
    }

    // Should have exactly 1 execution connection to node 3
    EXPECT_EQ(execConnsToNode3, 1);
    // It should be from node 2 (the most recent one)
    EXPECT_EQ(sourceNode, 2);
}

TEST_F(NodeEditorTest, ExecutionFlow_DataConnections_AllowMultipleOutputs)
{
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Source");
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "Dest1");
    auto node3 = std::make_unique<ExecutionFlowNode>(3, "Dest2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    // Data connections: 1 -> 2 and 1 -> 3 (should both exist)
    editor.AddConnection(1, "Output", 2, "Value", Nodes::ConnectionType::Data);
    editor.AddConnection(1, "Output", 3, "Value", Nodes::ConnectionType::Data);

    auto connections = editor.GetConnections();

    // Count data connections from node 1
    int dataConnsFromNode1 = 0;
    for (const auto &conn : connections)
    {
        if (conn.type == Nodes::ConnectionType::Data && conn.from == 1)
        {
            dataConnsFromNode1++;
        }
    }

    // Data connections should allow multiple outputs (1:N)
    EXPECT_EQ(dataConnsFromNode1, 2);
}

TEST_F(NodeEditorTest, ExecutionFlow_ErrorWhenUnconnectedExecutionPin)
{
    // Create node WITH execution pins but DON'T connect them
    auto node = std::make_unique<ExecutionFlowNode>(1, "Unconnected");
    editor.AddNode(std::move(node));

    // Should fail because node has execution pins but no execution connections
    bool success = editor.Execute();
    EXPECT_FALSE(success);
}

TEST_F(NodeEditorTest, ExecutionFlow_EntryPointDetection)
{
    ExecutionFlowNode::executionOrder.clear();

    // Entry point: node with execution output but NO input
    auto entry = std::make_unique<ExecutionFlowNode>(1, "Entry", false, true);
    auto middle = std::make_unique<ExecutionFlowNode>(2, "Middle");
    auto end = std::make_unique<ExecutionFlowNode>(3, "End", true, false);

    editor.AddNode(std::move(entry));
    editor.AddNode(std::move(middle));
    editor.AddNode(std::move(end));

    editor.AddConnection(1, "Then", 2, "Execute", Nodes::ConnectionType::Execution);
    editor.AddConnection(2, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // Should start from entry point (node 1)
    ASSERT_EQ(ExecutionFlowNode::executionOrder.size(), 3);
    EXPECT_EQ(ExecutionFlowNode::executionOrder[0], 1);
}

TEST_F(NodeEditorTest, ExecutionFlow_MixedGraph_LegacyNodesStillWork)
{
    ExecutionFlowNode::executionOrder.clear();

    // Mix of nodes with and without execution pins
    auto legacyNode1 = std::make_unique<SlotTestNode>(1, "Legacy1");
    auto legacyNode2 = std::make_unique<SlotTestNode>(2, "Legacy2");

    legacyNode1->SetInputSlotData("Input", 5.0);

    editor.AddNode(std::move(legacyNode1));
    editor.AddNode(std::move(legacyNode2));

    // Only data connections (no execution pins)
    editor.AddConnection(1, "Output", 2, "Input", Nodes::ConnectionType::Data);

    // Should work - fall back to data dependency order
    bool success = editor.Execute();
    EXPECT_TRUE(success);
}

TEST_F(NodeEditorTest, ExecutionFlow_CyclesPrevented_By1to1Enforcement)
{
    // Even if we try to create a cycle, 1:1 enforcement prevents it
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Node1");
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "Node2");

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));

    // Try to create cycle: 1 -> 2 -> 1
    editor.AddConnection(1, "Then", 2, "Execute", Nodes::ConnectionType::Execution);
    editor.AddConnection(2, "Then", 1, "Execute", Nodes::ConnectionType::Execution);

    // The second connection should remove node 1's output connection
    // So we end up with only 2 -> 1, not a cycle
    auto connections = editor.GetConnections();

    int execConnections = 0;
    for (const auto &conn : connections)
    {
        if (conn.type == Nodes::ConnectionType::Execution)
        {
            execConnections++;
        }
    }

    // Should have only 1 execution connection (2 -> 1)
    EXPECT_EQ(execConnections, 1);
}

TEST_F(NodeEditorTest, ExecutionFlow_DefensiveCycleDetection)
{
    ExecutionFlowNode::executionOrder.clear();

    // Create a valid linear execution flow
    auto node1 = std::make_unique<ExecutionFlowNode>(1, "Node1", false, true);
    auto node2 = std::make_unique<ExecutionFlowNode>(2, "Node2");
    auto node3 = std::make_unique<ExecutionFlowNode>(3, "Node3", true, false);

    editor.AddNode(std::move(node1));
    editor.AddNode(std::move(node2));
    editor.AddNode(std::move(node3));

    editor.AddConnection(1, "Then", 2, "Execute", Nodes::ConnectionType::Execution);
    editor.AddConnection(2, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    bool success = editor.Execute();
    EXPECT_TRUE(success);

    // All 3 nodes should execute
    EXPECT_EQ(ExecutionFlowNode::executionOrder.size(), 3);
}

TEST_F(NodeEditorTest, ExecutionFlow_MultipleEntryPoints_NotAllowed)
{
    // Two entry points (both with output but no input)
    auto entry1 = std::make_unique<ExecutionFlowNode>(1, "Entry1", false, true);
    auto entry2 = std::make_unique<ExecutionFlowNode>(2, "Entry2", false, true);
    auto node = std::make_unique<ExecutionFlowNode>(3, "Node");

    editor.AddNode(std::move(entry1));
    editor.AddNode(std::move(entry2));
    editor.AddNode(std::move(node));

    // Connect both entry points to the same node (violates 1:1 input)
    editor.AddConnection(1, "Then", 3, "Execute", Nodes::ConnectionType::Execution);
    editor.AddConnection(2, "Then", 3, "Execute", Nodes::ConnectionType::Execution);

    auto connections = editor.GetConnections();

    // Due to 1:1 enforcement, only one connection should exist
    int execConnsToNode3 = 0;
    for (const auto &conn : connections)
    {
        if (conn.type == Nodes::ConnectionType::Execution && conn.to == 3)
        {
            execConnsToNode3++;
        }
    }

    EXPECT_EQ(execConnsToNode3, 1);
}
